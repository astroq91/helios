require("lua.helios")

function on_start()
  RootEntity:get_components():get_physics_body().restitution = 1
end
