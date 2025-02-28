require("lua.helios")

local entities = {}

local MAX_HEIGHT = 1000
local ENT_COUNT = 2000
 

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
        local mesh = components:add_mesh()

        local scale = i / ENT_COUNT * MAX_HEIGHT
        transform.scale.y = scale
        transform.position.y = scale / 2

        mesh:load_geometry("Cube")
        mesh.tint_color = Vec4.new(1 - i / ENT_COUNT, i / ENT_COUNT, 0, 1)

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


function on_update(ts)
  
end
