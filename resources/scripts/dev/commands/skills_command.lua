local mm = require "mmbindings"
local command_utils = require "dev.commands.command_utils"
local utils = require "utils"

local function set_skill(skill_name, level, mastery_name, char_index)
    char_index = command_utils.character_or_current(char_index)
    mm.party.set_character_info(char_index, {
        skill = { id = mm.SkillType[skill_name], level = tonumber(level), mastery = mm.SkillMastery[mastery_name] }
    })
    local info = mm.party.get_character_info(char_index, { "name" })
    return info.name.." "..skill_name.." skill updated", true
end

local function set_skill_mastery(skill_name, mastery_name, char_index)
    char_index = command_utils.character_or_current(char_index)
    mm.party.set_character_info(char_index, {
        skill = { id = mm.SkillType[skill_name], mastery = mm.SkillMastery[mastery_name] }
    })
    local info = mm.party.get_character_info(char_index, { "name" })
    return info.name.." "..skill_name.." skill updated", true
end

local function set_skill_level(skill_name, level, char_index)
    char_index = command_utils.character_or_current(char_index)
    mm.party.set_character_info(char_index, {
        skill = { id = mm.SkillType[skill_name], level = tonumber(level) }
    })
    local info = mm.party.get_character_info(char_index, { "name" })
    return info.name.." "..skill_name.." skill updated", true
end

local function get_skills()
    local count = mm.party.get_party_size()
    local message = "Party Skills\n\n"
    for i = 1, count do
        local info = mm.party.get_character_info(i, { "name", "skills" })
        message = message..info.name..": \n\n"
        for skill_id, skill_value in pairs(info.skills) do
            local skill_name = utils.enum_to_string(mm.SkillType, skill_id)
            local skill_mastery_name = utils.enum_to_string(mm.SkillMastery, skill_value.mastery)
            message = message..skill_name..": "..skill_mastery_name.." - Level "..skill_value.level.."\n"
        end
    end
    return message, true
end

local function get_skill_mastery(skills, skill_id)
    for name, skill in pairs(skills) do
        if skill.id == skill_id then
            return skill.mastery
        end
    end
    return mm.SkillMastery.None
end

local function learn_all_skills()
    local count = mm.party.get_party_size()
    for char_index = 1, count do
        local info = mm.party.get_character_info(char_index, { "skills", "class" })
        print(info.class)
        for skill_name, skill_id in pairs(mm.SkillType) do
            if mm.game.can_class_learn(info.class, skill_id) then
                if get_skill_mastery(info.skills, skill_id) == mm.SkillMastery.None then
                    mm.party.set_character_info(char_index, {
                        skill = { id = skill_id, mastery = mm.SkillMastery.Novice, level = 1 }
                    })
                end
            end
        end
    end

    return "", true
end

local subcommands = {
    set = set_skill,
    level = set_skill_level,
    mastery = set_skill_mastery,
    get = get_skills,
    learn_all = learn_all_skills,
    default = get_skills
}

return {
    name = "skills",
    description = "Modify character skills",
    callback = subcommands
}