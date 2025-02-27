require("lua.helios")

function on_start()

  for i=-500, 500, 4 do
    local ent = Entities:create_entity("Entity " .. tostring(i + 50))
    local components = ent:get_components()

    local transform = components:add_transform()
    transform.position.x = i
    local scale = math.random(1, 100)
    transform.scale.y = scale
    transform.position.y = scale / 2

    local mesh = components:add_mesh()
    mesh:load_geometry("Cube")
  end
end
