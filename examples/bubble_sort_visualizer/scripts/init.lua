require("lua.helios")

local entities = {}

local MAX_HEIGHT = 1000
local ENT_COUNT = 1000
local ENT_DISTANCE = 4

function on_start()

  for i=0, ENT_COUNT, ENT_DISTANCE do
    local ent = Entities:create_entity("Entity " .. tostring(i + 50))
    local components = ent:get_components()

    local transform = components:add_transform()
    transform.position.x = i - ENT_COUNT / 2
    local scale = i / ENT_COUNT / ENT_DISTANCE * MAX_HEIGHT
    transform.scale.y = scale
    transform.position.y = scale / 2

    local mesh = components:add_mesh()
    mesh:load_geometry("Cube")

    mesh.tint_color = Vec4.new(1 - i / ENT_COUNT * 1, i / ENT_COUNT * 1, 0, 1)

    table.insert(entities, ent)
  end
end

function on_update(ts)
  
end
