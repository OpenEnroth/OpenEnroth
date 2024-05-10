local Game = require "bindings.game"
local CommandUtilities = require "dev.commands.command_utils"

local function setSkill(skillName, level, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = {
            id = stringToEnum(Game.SkillType, skillName),
            level = tonumber(level),
            mastery = stringToEnum(Game.SkillMastery, masteryName)
        }
    })
    local info = Game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillMastery(skillName, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = { id = stringToEnum(Game.SkillType, skillName), mastery = stringToEnum(Game.SkillMastery, masteryName) }
    })
    local info = Game.party.getCharacterInfo(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillLevel(skillName, level, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        skill = { id = stringToEnum(Game.SkillType, skillName), level = tonumber(level) }
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
            local skillName = enumToString(Game.SkillType, skillId)
            local skillMasteryName = enumToString(Game.SkillMastery, skillValue.mastery)
            message = message ..
                skillName .. ": " .. skillMasteryName .. " - Level " .. tostring(skillValue.level) .. "\n"
        end
    end
    return message, true
end

---@param skills table<SkillType, SkillEntry>
---@param skillId SkillType
---@return SkillMastery
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
        for _, skillType in pairs(Game.SkillType) do
            if Game.misc.canClassLearn(info.class, skillType) then
                if getSkillMastery(info.skills, skillType) == Game.SkillMastery.None then
                    Game.party.setCharacterInfo(charIndex, {
                        skill = { id = skillType, mastery = Game.SkillMastery.Novice, level = 1 }
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
