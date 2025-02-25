require("lua.helios")

function on_start()
  RootEntity:get_components():get_rigid_body().restitution = 1
end
