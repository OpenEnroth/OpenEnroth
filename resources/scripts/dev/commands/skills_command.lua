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
    {
        name = "set",
        callback = setSkill,
        params = {
            {
                name = "skill",
                type = "enum",
                enumValues = Game.SkillType,
                description = "Skill to set."
            },
            {
                name = "level",
                type = "number",
                description = "Level of the skill."
            },
            {
                name = "mastery",
                type = "enum",
                enumValues = Game.SkillMastery,
                description = "Mastery of the skill."
            },
            {
                name = "char",
                type = "characterIndex",
                description = "Character index to set skill for."
            }
        },
        description = "Sets the specified skill for the character."
    },
    {
        name = "level",
        callback = setSkillLevel,
        params = {
            {
                name = "skill",
                type = "enum",
                enumValues = Game.SkillType,
                description = "Skill to set level for."
            },
            { name = "level", type = "number",         description = "Level of the skill." },
            { name = "char",  type = "characterIndex", description = "Character index to set skill level for." }
        },
        description = "Sets the specified skill level for the character."
    },
    {
        name = "mastery",
        callback = setSkillMastery,
        params = {
            {
                name = "skill",
                type = "enum",
                enumValues = Game.SkillType,
                optional = false,
                description = "Skill to set mastery for."
            },
            {
                name = "mastery",
                type = "enum",
                enumValues = Game.SkillMastery,
                optional = false,
                description = "Mastery of the skill."
            },
            { name = "char", type = "characterIndex", optional = true, description = "Character index to set skill mastery for." }
        },
        description =
        "Sets the specified skill mastery for the character."
    },
    {
        name = "get",
        callback = getSkills,
        description = "Shows all skills of all characters in the party."
    },
    {
        name = "learn_all",
        callback = learnAllSkills,
        description = "Learn all skills available for each character in the party."
    },
}

return {
    name = "skills",
    description = "Modify character skills",
    subCommands = subCommands
}
