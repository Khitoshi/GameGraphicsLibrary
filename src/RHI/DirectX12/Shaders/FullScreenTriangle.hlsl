struct VSOutput
{
    noperspective float4 Position : SV_Position;
    noperspective float2 UV : TEXCOORD;
};

VSOutput FullScreenTriangleVS(in uint VertexIdx : SV_VertexID)
{
    VSOutput output;

    // TODO: write fullscreen triangle code.
    output.Position = float4(0, 0, 0, 1);
    output.UV = float2(0, 0);
    return output;
}