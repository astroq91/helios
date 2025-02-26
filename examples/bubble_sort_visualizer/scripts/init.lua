require("lua.helios")

function on_start()
  local ent = Entities:create_entity("Cool entity")
  local components = ent:get_components()
  if not components then
    print("Oops")
  end
  components:add_transform()
  local mesh = components:add_mesh()

 mesh:load_geometry("Cube")
end
