#version 450

layout (binding = ) uniform objects_data
{
        mat3 model_matrix, view_matrix, projection_matrix;
} object_data;

void main()
{
        gl_Position = projection_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
}       