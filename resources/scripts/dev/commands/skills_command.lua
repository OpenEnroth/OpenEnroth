local game = requireGame()
local CommandUtilities = require "dev.commands.command_utils"
local Utilities = require "utils"

local function setSkill(skillName, level, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    game.party.setCharacterInfo(charIndex, {
        skill = { id = game.SkillType[skillName], level = tonumber(level), mastery = game.SkillMastery[masteryName] }
    })
    local info = game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillMastery(skillName, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    game.party.setCharacterInfo(charIndex, {
        skill = { id = game.SkillType[skillName], mastery = game.SkillMastery[masteryName] }
    })
    local info = game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillLevel(skillName, level, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    game.party.setCharacterInfo(charIndex, {
        skill = { id = game.SkillType[skillName], level = tonumber(level) }
    })
    local info = game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

---@return string
---@return boolean
local function getSkills()
    local count = game.party.getPartySize()
    local message = "Party Skills\n\n"
    for i = 1, count do
        local info = game.party.getCharacterInfo(i, { "name", "skills" })
        message = message .. info.name .. ": \n\n"
        for skillId, skillValue in pairs(info.skills) do
            local skillName = Utilities.enumToString(game.SkillType, skillId)
            local skillMasteryName = Utilities.enumToString(game.SkillMastery, skillValue.mastery)
            message = message ..
                skillName .. ": " .. skillMasteryName .. " - Level " .. tostring(skillValue.level) .. "\n"
        end
    end
    return message, true
end

---@param skills table<integer, SkillEntry>
---@param skillId integer
---@return integer
local function getSkillMastery(skills, skillId)
    for _, skill in pairs(skills) do
        if skill.id == skillId then
            return skill.mastery
        end
    end
    return game.SkillMastery.None
end

local function learnAllSkills()
    local count = game.party.getPartySize()
    for charIndex = 1, count do
        local info = game.party.getCharacterInfo(charIndex, { "skills", "class" })
        for _, skillId in pairs(game.SkillType) do
            if game.misc.canClassLearn(info.class, skillId) then
                if getSkillMastery(info.skills, skillId) == game.SkillMastery.None then
                    game.party.setCharacterInfo(charIndex, {
                        skill = { id = skillId, mastery = game.SkillMastery.Novice, level = 1 }
                    })
                end
            end
        end
    end

    return "", true
end

local subCommands = {
    set = setSkill,
    level = setSkillLevel,
    mastery = setSkillMastery,
    get = getSkills,
    learn_all = learnAllSkills,
    default = getSkills
}

return {
    name = "skills",
    description = "Modify character skills",
    callback = subCommands
}
