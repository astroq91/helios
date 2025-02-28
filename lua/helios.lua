require("lua.key_codes")

---@class vec3
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field new fun() Creates a default vec3
---@field new fun(x: number, y: number, z: number) Creates a vec3 with given values
Vec3 = Vec3

---@class vec4
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component
---@field new fun() Creates a default vec4
---@field new fun(x: number, y: number, z: number, w: number) Creates a vec4 with given values
Vec4 = Vec4

---@class quat 
---@field x number The x component
---@field y number The y component
---@field z number The z component
---@field w number The w component
Quat = Quat

---@class Name
---@field name string The name of the entity
Name = Name

---@class Camera
---@field fov_y number The y-fov
---@field near number  The near value
---@field far number   The far value
Camera = Camera

---@class DirectionalLight
---@field direction vec3 The direction
---@field ambient vec3   The ambient color of the light 
---@field diffuse vec3   The diffuse color of the light 
---@field specular vec3  The specular color of the light
DirectionalLight = DirectionalLight

---@class PointLight
---@field position vec3     The position of the light
---@field constant number   The constant co-efficient
---@field linear number     The linear co-efficient
---@field quadratic number  The quadratic co-efficient
---@field ambient vec3      The ambient color of the light
---@field diffuse vec3      The diffuse color of the light
---@field specular vec3     The specular color of the light
PointLight = PointLight

---@class RigidBody 
---@field mass number The mass
---@field static_friction number The static friction
---@field dynamic_friction number The dynamic friction
---@field restitution number The restitution
RigidBody = RigidBody

---@class Mesh
---@field load_geometry fun(self: Mesh, name: string) Loads a geometry from a relative (based on project path), or absolute path.
---                                                   You can also specify default geometries like "Cube"
---@field load_material fun(self: Mesh, name: string) Loads a material from a relative (based on project path), or aboslute path.
---@field tint_color vec4 The tinting of the mesh
Mesh = Mesh

---@class Components
---Contains functions for accessing the components
---@field get_transform fun():Transform Gets the transform
---@field get_name fun():Name Gets the name
---@field get_camera fun():Camera Gets the camera
---@field get_directional_light fun():DirectionalLight Gets the directional light
---@field get_point_light fun():PointLight Gets the point light
---@field get_rigid_body fun():RigidBody Gets the rigid body 
---@field get_mesh fun():Mesh Gets the mesh 
---
---@field add_transform fun():Transform Adds a transform component
---@field add_camera fun():Camera Adds a camera component
---@field add_directional_light fun():DirectionalLight Adds a directional light component
---@field add_point_light fun():PointLight Adds a point light component
---@field add_rigid_body fun():RigidBody Adds a rigid body component 
---@field add_mesh fun():Mesh Adds a mesh component 
Components = Components

---@class Entity
---An entity object
---@field get_components fun():Components Gets the components object
RootEntity = RootEntity

---@class Entities
---The collection of entities
---@field create_entity fun(self: Entities, name: string):Entity Creates a new entity with the given name
Entities = Entities

---@class Transform
---@field position vec3 The position
---@field rotation vec3 The rotation 
---@field scale vec3    The scale 
Transform = Transform

---@class Input
---@field is_key_pressed fun(KeyCode):boolean  True if the key is pressed
---@field is_key_released fun(KeyCode):boolean True if the key was released
Input = Input

