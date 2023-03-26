
void sort(unsigned char* values, int size)
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = i + 1; j < size; ++j)
        {
            if (values[i] > values[j])
            {
                char temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }
}

__kernel void Main(__global const unsigned char* input, int w, int h, int ker, __global unsigned char* output)
{
    int x = get_global_id(0); 
    int y = get_global_id(1); 

    int radios = ker / 2;
    unsigned char values[256];
    int iter = 0;
    for (int i = -radios; i < radios; ++i)
    {
        for (int j = -radios; j < radios; ++j)
        {
            int ry = y + i;
            int rx = x + j * 3;

            if (rx < 0) rx = x % 3;
            if (ry < 0) ry = 0;
            if (w * 3 <= rx) rx = w * 3 - 1;
            if (h <= ry) ry = h - 1;

            values[iter++] = input[rx + ry * w * 3];
        }
    }

    sort(values, ker * ker);

    output[y * w * 3 + x] = values[ker * ker / 2];
}