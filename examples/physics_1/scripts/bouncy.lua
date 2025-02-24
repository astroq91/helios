require("lua.helios")

function on_start()
  Components:get_rigid_body().restitution = 1
end
