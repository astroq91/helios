require("lua.helios")
---@expose
CameraEntity = Entity.new()

function on_start()
  if CameraEntity:is_valid() then
    print(CameraEntity:get_components():get_name().name)
  end
end


