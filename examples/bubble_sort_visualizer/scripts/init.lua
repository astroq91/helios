require("lua.helios")

local entities = {}

local MAX_HEIGHT = 20
local ENT_COUNT = 50
local sort_done = false
local swapped = false
local current_index = 2
local last_sorted_index = ENT_COUNT

math.randomseed(os.time())
local function shuffle_in_place(t)
    for i = #t, 2, -1 do
        local j = math.random(i)
        t[i], t[j] = t[j], t[i]
    end
end

function on_start()
    -- Create the entities with sizes in acsending order
    for i = 1, ENT_COUNT do
        local ent = Entities:create_entity("Entity " .. tostring(i))
        local components = ent:get_components()
        local transform = components:add_transform()
        local mesh_renderer = components:add_mesh_renderer()

        local scale = i / ENT_COUNT * MAX_HEIGHT
        transform.scale.y = scale
        transform.position.y = scale / 2

        mesh_renderer:load_mesh("Cube")
        mesh_renderer.tint_color = Vec4.new(1 - i / ENT_COUNT, i / ENT_COUNT, 0, 1)

        table.insert(entities, ent)
    end

    shuffle_in_place(entities)

    -- Update the x position to reflect the new order
    for k, v in pairs(entities) do
        local components = v:get_components()
        local transform = components:get_transform()

        transform.position.x = k - ENT_COUNT / 2
    end
end

function swap_if_bigger(i, j)
    local next_entity_transform = entities[i]:get_components():get_transform()
    local curr_entity_transform = entities[j]:get_components():get_transform()

    if curr_entity_transform.scale.y > next_entity_transform.scale.y then
        local temp = entities[i]
        entities[i] = entities[j]
        entities[j] = temp

        temp = next_entity_transform.position.x
        next_entity_transform.position.x = curr_entity_transform.position.x
        curr_entity_transform.position.x = temp

        return true
    end
    return false
end

function on_update(ts)
    if sort_done then
        return
    end

    local next_entity_transform = entities[current_index]:get_components():get_transform()
    local curr_entity_transform = entities[current_index-1]:get_components():get_transform()

    if swap_if_bigger(current_index, current_index-1) then
        swapped = true
    end

    if current_index == last_sorted_index then
        if swapped then
            swapped = false
        else
            sort_done = true
        end
        last_sorted_index = last_sorted_index - 1
        current_index = 2
    else
        current_index = current_index + 1
    end
end
