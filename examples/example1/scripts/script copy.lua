require("lua.helios")

---@expose
CubeEntity = Entity.new()

function on_start()
  print("on_start!")
end

function on_update(ts)
  local speed = 8.0

  local transform = RootEntity:get_components():get_transform()
  print(tostring(transform.scale.x));
end



