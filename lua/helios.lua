require("lua.key_codes")

---@class vec3
---@field x number The x component
---@field y number The y component
---@field z number The z component
vec3 = vec3 or {}

---@class vec4
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component
vec4 = vec4 or {}

---@class quat 
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component
quat = quat or {}

---@class Name
---@field name string The name of the entity
Name = Name or {}

---@class Camera
---@field fov_y number The y-fov
---@field near number  The near value
---@field far number   The far value
Camera = Camera or {}

---@class DirectionalLight
---@field direction vec3 The direction
---@field ambient vec3   The ambient color of the light 
---@field diffuse vec3   The diffuse color of the light 
---@field specular vec3  The specular color of the light
DirectionalLight = DirectionalLight or {}

---@class PointLight
---@field position vec3     The position of the light
---@field constant number   The constant co-efficient
---@field linear number     The linear co-efficient
---@field quadratic number  The quadratic co-efficient
---@field ambient vec3      The ambient color of the light
---@field diffuse vec3      The diffuse color of the light
---@field specular vec3     The specular color of the light
PointLight = PointLight or {}

---@class RigidBody 
---@field mass number The mass
---@field static_friction number The static friction
---@field dynamic_friction number The dynamic friction
---@field restitution number The restitution
RigidBody = RigidBody or {}

---@class Components
---Contains functions for accessing the components
---@field get_transform fun():Transform Gets the transform
---@field get_name fun():Name Gets the transform
---@field get_camera fun():Camera Gets the transform
---@field get_directional_light fun():DirectionalLight Gets the transform
---@field get_point_light fun():PointLight Gets the transform
---@field get_rigid_body fun():RigidBody Gets the rigid body 
Components = Components or {}

---@class Transform
---@field position vec3 The position
---@field rotation vec3 The rotation 
---@field scale vec3    The scale 
Transform = Transform or {}

---@class Input
---@field is_key_pressed fun(KeyCode):boolean  True if the key is pressed
---@field is_key_released fun(KeyCode):boolean True if the key was released
Input = Input or {}
