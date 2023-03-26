__kernel void Main(__global const unsigned char* Input, __global unsigned char* Output)
{
    int x = get_global_id(0);
    Output[x] = 255 - Input[x];
}