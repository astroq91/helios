require("lua.key_codes")

---@class vec3
---@field x number The x component
---@field y number The y component
---@field z number The z component

---@class vec4
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component

---@class quat 
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component

---@class Name
---@field name string The name of the entity

---@class Camera
---@field fov_y number The y-fov
---@field near number  The near value
---@field far number   The far value

---@class DirectionalLight
---@field direction vec3 The direction
---@field ambient vec3   The ambient color of the light 
---@field diffuse vec3   The diffuse color of the light 
---@field specular vec3  The specular color of the light

---@class PointLight
---@field position vec3     The position of the light
---@field constant number   The constant co-efficient
---@field linear number     The linear co-efficient
---@field quadratic number  The quadratic co-efficient
---@field ambient vec3      The ambient color of the light
---@field diffuse vec3      The diffuse color of the light
---@field specular vec3     The specular color of the light

---@class RigidBody 
---@field mass number The mass
---@field static_friction number The static friction
---@field dynamic_friction number The dynamic friction
---@field restitution number The restitution

---@class Components
---Contains functions for accessing the components
---@field get_transform fun():Transform Gets the transform
---@field get_name fun():Name Gets the transform
---@field get_camera fun():Camera Gets the transform
---@field get_directional_light fun():DirectionalLight Gets the transform
---@field get_point_light fun():PointLight Gets the transform
---@field get_rigid_body fun():RigidBody Gets the rigid body 

---@class Entity
---An entity object
---@field get_components fun():Components Gets the components object
RootEntity = RootEntity or {}

---@class Entities
---The collection of entities
---@field create_entity fun(self: Entities, name: string):Entity Creates a new entity with the given name
Entities = Entities or {}

---@class Transform
---@field position vec3 The position
---@field rotation vec3 The rotation 
---@field scale vec3    The scale 

---@class Input
---@field is_key_pressed fun(KeyCode):boolean  True if the key is pressed
---@field is_key_released fun(KeyCode):boolean True if the key was released
Input = Input or {}
