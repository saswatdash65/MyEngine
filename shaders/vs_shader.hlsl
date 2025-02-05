float2 vertices[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(0.5, -0.5)
};

float4 main(uint id : SV_VertexID) : SV_POSITION {
    return float4(vertices[id], 0.0, 1.0);
}
