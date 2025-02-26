require("lua.helios")

function on_update()
 if Input.is_key_pressed(Key.Space) then
  Entities:create_entity("New entity!!")
 end
end
