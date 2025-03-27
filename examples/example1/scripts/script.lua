require("lua.helios")

---@expose
CubeEntity = Entity.new()

function on_start()
  print("on_start!")
end

function on_update(ts)
  local speed = 8.0

  local transform = RootEntity:get_components():get_transform()

  if transform then
    if Input:is_key_pressed(Key.W) then
      transform.position.z = transform.position.z - ts * speed
    end
    if Input:is_key_pressed(Key.A) then
      transform.position.x = transform.position.x - ts * speed
    end
    if Input:is_key_pressed(Key.S) then
      transform.position.z = transform.position.z + ts * speed
    end
    if Input:is_key_pressed(Key.D)  then
      transform.position.x = transform.position.x + ts * speed
    end
    if Input:is_key_pressed(Key.Space)  then
      transform.position.y = transform.position.y + ts * speed
    end
    if Input:is_key_pressed(Key.LeftShift)  then
      transform.position.y = transform.position.y - ts * speed
    end

    transform.position.x = transform.position.x + ts;
  end


end



