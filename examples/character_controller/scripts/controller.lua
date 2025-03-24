require("lua.helios")
---@expose
CameraEntity = Entity.new()

function on_update()
  if Input:is_key_pressed(Key.W) then
    local force = Vec3.new(0, 0, -1)
    RootEntity:get_components():get_rigid_body():add_force(force)
  end

end
