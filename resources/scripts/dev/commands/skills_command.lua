local MM = require "mmbindings"
local CommandUtilities = require "dev.commands.command_utils"
local Utilities = require "utils"

local function setSkill(skillName, level, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    MM.party.set_character_info(charIndex, {
        skill = { id = MM.SkillType[skillName], level = tonumber(level), mastery = MM.SkillMastery[masteryName] }
    })
    local info = MM.party.get_character_info(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillMastery(skillName, masteryName, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    MM.party.set_character_info(charIndex, {
        skill = { id = MM.SkillType[skillName], mastery = MM.SkillMastery[masteryName] }
    })
    local info = MM.party.get_character_info(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

local function setSkillLevel(skillName, level, charIndex)
    charIndex = CommandUtilities.characterOrCurrent(charIndex)
    MM.party.set_character_info(charIndex, {
        skill = { id = MM.SkillType[skillName], level = tonumber(level) }
    })
    local info = MM.party.get_character_info(charIndex, { "name" })
    return info.name .. " " .. skillName .. " skill updated", true
end

---@return string
---@return boolean
local function getSkills()
    local count = MM.party.get_party_size()
    local message = "Party Skills\n\n"
    for i = 1, count do
        local info = MM.party.get_character_info(i, { "name", "skills" })
        message = message .. info.name .. ": \n\n"
        for skillId, skillValue in pairs(info.skills) do
            local skillName = Utilities.enumToString(MM.SkillType, skillId)
            local skillMasteryName = Utilities.enumToString(MM.SkillMastery, skillValue.mastery)
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
    return MM.SkillMastery.None
end

local function learnAllSkills()
    local count = MM.party.get_party_size()
    for charIndex = 1, count do
        local info = MM.party.get_character_info(charIndex, { "skills", "class" })
        for _, skillId in pairs(MM.SkillType) do
            if MM.game.can_class_learn(info.class, skillId) then
                if getSkillMastery(info.skills, skillId) == MM.SkillMastery.None then
                    MM.party.set_character_info(charIndex, {
                        skill = { id = skillId, mastery = MM.SkillMastery.Novice, level = 1 }
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
