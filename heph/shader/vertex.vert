#version 450

layout (push_constants) uniform l0
{
        mat4 view_matrix, projection_matrix;
} transform_matrices;

layout (binding = ) uniform l1
{
        uint nobjects;  
        Object objects[];
} object_buffer;

void main()
{
        mat3 model_matrix = object_buffer.objects[gl_InstanceIndex].model_matrix;
        gl_Position = vec4(vertex_position, 1.0) * model_matrix * view_matrix * projection_matrix;
}