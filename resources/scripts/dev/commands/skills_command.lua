local Game = require "bindings.game"
local CommandUtilities = require "dev.commands.command_utils"
local Utilities = require "utils"

local function setSkill(skillName, level, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = { id = Game.SkillType[skillName], level = tonumber(level), mastery = Game.SkillMastery[masteryName] }
    })
    local info = Game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillMastery(skillName, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = { id = Game.SkillType[skillName], mastery = Game.SkillMastery[masteryName] }
    })
    local info = Game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillLevel(skillName, level, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = { id = Game.SkillType[skillName], level = tonumber(level) }
    })
    local info = Game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

---@return string
---@return boolean
local function getSkills()
    local count = Game.party.getPartySize()
    local message = "Party Skills\n\n"
    for i = 1, count do
        local info = Game.party.getCharacterInfo(i, { "name", "skills" })
        message = message .. info.name .. ": \n\n"
        for skillId, skillValue in pairs(info.skills) do
            local skillName = Utilities.enumToString(Game.SkillType, skillId)
            local skillMasteryName = Utilities.enumToString(Game.SkillMastery, skillValue.mastery)
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
    return Game.SkillMastery.None
end

local function learnAllSkills()
    local count = Game.party.getPartySize()
    for charIndex = 1, count do
        local info = Game.party.getCharacterInfo(charIndex, { "skills", "class" })
        for _, skillId in pairs(Game.SkillType) do
            if Game.misc.canClassLearn(info.class, skillId) then
                if getSkillMastery(info.skills, skillId) == Game.SkillMastery.None then
                    Game.party.setCharacterInfo(charIndex, {
                        skill = { id = skillId, mastery = Game.SkillMastery.Novice, level = 1 }
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
