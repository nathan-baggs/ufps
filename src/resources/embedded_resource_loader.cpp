#include "resources/embedded_resource_loader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/error.h"

namespace
{

constexpr const std::uint8_t copper_albedo[] = {
#embed "../../assets/textures/copper_albedo.png"
};


constexpr const std::uint8_t diamond_floor_specular[] = {
#embed "../../assets/textures/diamond_floor_specular.png"
};


constexpr const std::uint8_t copper_normal[] = {
#embed "../../assets/textures/copper_normal.png"
};


constexpr const std::uint8_t diamond_floor_normal[] = {
#embed "../../assets/textures/diamond_floor_normal.png"
};


constexpr const std::uint8_t rusty_metal_normal[] = {
#embed "../../assets/textures/rusty_metal_normal.png"
};


constexpr const std::uint8_t diamond_floor_albedo[] = {
#embed "../../assets/textures/diamond_floor_albedo.png"
};


constexpr const std::uint8_t rusty_metal_albedo[] = {
#embed "../../assets/textures/rusty_metal_albedo.png"
};


constexpr const std::uint8_t rusty_metal_specular[] = {
#embed "../../assets/textures/rusty_metal_specular.png"
};


constexpr const std::uint8_t copper_specular[] = {
#embed "../../assets/textures/copper_specular.png"
};


constexpr const std::uint8_t simple_vert[] = {
#embed "../../assets/shaders/simple.vert"
};


constexpr const std::uint8_t light_pass_frag[] = {
#embed "../../assets/shaders/light_pass.frag"
};


constexpr const std::uint8_t gbuffer_vert[] = {
#embed "../../assets/shaders/gbuffer.vert"
};


constexpr const std::uint8_t gbuffer_frag[] = {
#embed "../../assets/shaders/gbuffer.frag"
};


constexpr const std::uint8_t simple_frag[] = {
#embed "../../assets/shaders/simple.frag"
};


constexpr const std::uint8_t light_pass_vert[] = {
#embed "../../assets/shaders/light_pass.vert"
};


constexpr const std::uint8_t MetalPlate03_Height[] = {
#embed "../../secret-assets/textures/MetalPlate03_Height.png"
};


constexpr const std::uint8_t DeatilsBG_Normal[] = {
#embed "../../secret-assets/textures/DeatilsBG_Normal.png"
};


constexpr const std::uint8_t Display_BaseColor[] = {
#embed "../../secret-assets/textures/Display_BaseColor.png"
};


constexpr const std::uint8_t Light02Red_Roughness[] = {
#embed "../../secret-assets/textures/Light02Red_Roughness.png"
};


constexpr const std::uint8_t Door01_BaseColor[] = {
#embed "../../secret-assets/textures/Door01_BaseColor.png"
};


constexpr const std::uint8_t Door02_BaseColor[] = {
#embed "../../secret-assets/textures/Door02_BaseColor.png"
};


constexpr const std::uint8_t Display_Height[] = {
#embed "../../secret-assets/textures/Display_Height.png"
};


constexpr const std::uint8_t Generator_Metallic[] = {
#embed "../../secret-assets/textures/Generator_Metallic.png"
};


constexpr const std::uint8_t Display_Normal[] = {
#embed "../../secret-assets/textures/Display_Normal.png"
};


constexpr const std::uint8_t Glass01_Metallic[] = {
#embed "../../secret-assets/textures/Glass01_Metallic.png"
};


constexpr const std::uint8_t T_Station01_Metalic[] = {
#embed "../../secret-assets/textures/T_Station01_Metalic.png"
};


constexpr const std::uint8_t Planet_BC[] = {
#embed "../../secret-assets/textures/Planet_BC.png"
};


constexpr const std::uint8_t Door03_Roughness[] = {
#embed "../../secret-assets/textures/Door03_Roughness.png"
};


constexpr const std::uint8_t Door02_Height[] = {
#embed "../../secret-assets/textures/Door02_Height.png"
};


constexpr const std::uint8_t Door02_Normal[] = {
#embed "../../secret-assets/textures/Door02_Normal.png"
};


constexpr const std::uint8_t MetalPlate01_Roughness[] = {
#embed "../../secret-assets/textures/MetalPlate01_Roughness.png"
};


constexpr const std::uint8_t MetalPlate02_BaseColor[] = {
#embed "../../secret-assets/textures/MetalPlate02_BaseColor.png"
};


constexpr const std::uint8_t Detail02_BaseColor[] = {
#embed "../../secret-assets/textures/Detail02_BaseColor.png"
};


constexpr const std::uint8_t Planet_R[] = {
#embed "../../secret-assets/textures/Planet_R.png"
};


constexpr const std::uint8_t Door03_BaseColor[] = {
#embed "../../secret-assets/textures/Door03_BaseColor.png"
};


constexpr const std::uint8_t Door01_AO[] = {
#embed "../../secret-assets/textures/Door01_AO.png"
};


constexpr const std::uint8_t Panel14Details_BaseColor[] = {
#embed "../../secret-assets/textures/Panel14Details_BaseColor.png"
};


constexpr const std::uint8_t Planet_N[] = {
#embed "../../secret-assets/textures/Planet_N.png"
};


constexpr const std::uint8_t Generator_Emissive[] = {
#embed "../../secret-assets/textures/Generator_Emissive.png"
};


constexpr const std::uint8_t Light02Red_Metallic[] = {
#embed "../../secret-assets/textures/Light02Red_Metallic.png"
};


constexpr const std::uint8_t Door01_Normal[] = {
#embed "../../secret-assets/textures/Door01_Normal.png"
};


constexpr const std::uint8_t Panel14Details_Emissive[] = {
#embed "../../secret-assets/textures/Panel14Details_Emissive.png"
};


constexpr const std::uint8_t DeatilsBG_BaseColor[] = {
#embed "../../secret-assets/textures/DeatilsBG_BaseColor.png"
};


constexpr const std::uint8_t Details01_BaseColor[] = {
#embed "../../secret-assets/textures/Details01_BaseColor.png"
};


constexpr const std::uint8_t Elevator_Metallic[] = {
#embed "../../secret-assets/textures/Elevator_Metallic.png"
};


constexpr const std::uint8_t MetalPlate03_Roughness[] = {
#embed "../../secret-assets/textures/MetalPlate03_Roughness.png"
};


constexpr const std::uint8_t Door03_Normal[] = {
#embed "../../secret-assets/textures/Door03_Normal.png"
};


constexpr const std::uint8_t Light01Blue_Metallic[] = {
#embed "../../secret-assets/textures/Light01Blue_Metallic.png"
};


constexpr const std::uint8_t Planet_AO[] = {
#embed "../../secret-assets/textures/Planet_AO.png"
};


constexpr const std::uint8_t T_Station01_Normal[] = {
#embed "../../secret-assets/textures/T_Station01_Normal.png"
};


constexpr const std::uint8_t Generator_Roughness[] = {
#embed "../../secret-assets/textures/Generator_Roughness.png"
};


constexpr const std::uint8_t Elevator_Normal[] = {
#embed "../../secret-assets/textures/Elevator_Normal.png"
};


constexpr const std::uint8_t Panel14Details_Height[] = {
#embed "../../secret-assets/textures/Panel14Details_Height.png"
};


constexpr const std::uint8_t Door01_Emissive[] = {
#embed "../../secret-assets/textures/Door01_Emissive.png"
};


constexpr const std::uint8_t T_Station01_Emission[] = {
#embed "../../secret-assets/textures/T_Station01_Emission.png"
};


constexpr const std::uint8_t Generator_AO[] = {
#embed "../../secret-assets/textures/Generator_AO.png"
};


constexpr const std::uint8_t Details03_Roughness[] = {
#embed "../../secret-assets/textures/Details03_Roughness.png"
};


constexpr const std::uint8_t Glass01_Roughness[] = {
#embed "../../secret-assets/textures/Glass01_Roughness.png"
};


constexpr const std::uint8_t MetalPlate02_Normal[] = {
#embed "../../secret-assets/textures/MetalPlate02_Normal.png"
};


constexpr const std::uint8_t Details01_Metallic[] = {
#embed "../../secret-assets/textures/Details01_Metallic.png"
};


constexpr const std::uint8_t Elevator_Roughness[] = {
#embed "../../secret-assets/textures/Elevator_Roughness.png"
};


constexpr const std::uint8_t MetalPlate02_Roughness[] = {
#embed "../../secret-assets/textures/MetalPlate02_Roughness.png"
};


constexpr const std::uint8_t Door03_Metallic[] = {
#embed "../../secret-assets/textures/Door03_Metallic.png"
};


constexpr const std::uint8_t Details03_Metallic[] = {
#embed "../../secret-assets/textures/Details03_Metallic.png"
};


constexpr const std::uint8_t Detail02_Normal[] = {
#embed "../../secret-assets/textures/Detail02_Normal.png"
};


constexpr const std::uint8_t Details01_Roughness[] = {
#embed "../../secret-assets/textures/Details01_Roughness.png"
};


constexpr const std::uint8_t MetalPlate02_Metallic[] = {
#embed "../../secret-assets/textures/MetalPlate02_Metallic.png"
};


constexpr const std::uint8_t Elevator_AO[] = {
#embed "../../secret-assets/textures/Elevator_AO.png"
};


constexpr const std::uint8_t Glass01_Normal[] = {
#embed "../../secret-assets/textures/Glass01_Normal.png"
};


constexpr const std::uint8_t Details01_Height[] = {
#embed "../../secret-assets/textures/Details01_Height.png"
};


constexpr const std::uint8_t Panel14Details_Metallic[] = {
#embed "../../secret-assets/textures/Panel14Details_Metallic.png"
};


constexpr const std::uint8_t Generator_Height[] = {
#embed "../../secret-assets/textures/Generator_Height.png"
};


constexpr const std::uint8_t T_Station01_Height[] = {
#embed "../../secret-assets/textures/T_Station01_Height.png"
};


constexpr const std::uint8_t MetalPlate03_BaseColor[] = {
#embed "../../secret-assets/textures/MetalPlate03_BaseColor.png"
};


constexpr const std::uint8_t Light02Red_Height[] = {
#embed "../../secret-assets/textures/Light02Red_Height.png"
};


constexpr const std::uint8_t SolarCell_Height[] = {
#embed "../../secret-assets/textures/SolarCell_Height.png"
};


constexpr const std::uint8_t Details02_AO[] = {
#embed "../../secret-assets/textures/Details02_AO.png"
};


constexpr const std::uint8_t Elevator_Height[] = {
#embed "../../secret-assets/textures/Elevator_Height.png"
};


constexpr const std::uint8_t SolarCell_Normal[] = {
#embed "../../secret-assets/textures/SolarCell_Normal.png"
};


constexpr const std::uint8_t Door02_AO[] = {
#embed "../../secret-assets/textures/Door02_AO.png"
};


constexpr const std::uint8_t Light01Blue_Roughness[] = {
#embed "../../secret-assets/textures/Light01Blue_Roughness.png"
};


constexpr const std::uint8_t Detail02_Roughness[] = {
#embed "../../secret-assets/textures/Detail02_Roughness.png"
};


constexpr const std::uint8_t MetalPlate01_Normal[] = {
#embed "../../secret-assets/textures/MetalPlate01_Normal.png"
};


constexpr const std::uint8_t Glass01_BaseColor[] = {
#embed "../../secret-assets/textures/Glass01_BaseColor.png"
};


constexpr const std::uint8_t T_Door03_AO[] = {
#embed "../../secret-assets/textures/T_Door03_AO.png"
};


constexpr const std::uint8_t Light02Red_Normal[] = {
#embed "../../secret-assets/textures/Light02Red_Normal.png"
};


constexpr const std::uint8_t Details03_Normal[] = {
#embed "../../secret-assets/textures/Details03_Normal.png"
};


constexpr const std::uint8_t Pipes01_Roughness[] = {
#embed "../../secret-assets/textures/Pipes01_Roughness.png"
};


constexpr const std::uint8_t Door01_Roughness[] = {
#embed "../../secret-assets/textures/Door01_Roughness.png"
};


constexpr const std::uint8_t MetalPlate01_Height[] = {
#embed "../../secret-assets/textures/MetalPlate01_Height.png"
};


constexpr const std::uint8_t Panel14Details_AO[] = {
#embed "../../secret-assets/textures/Panel14Details_AO.png"
};


constexpr const std::uint8_t Display_Emissive[] = {
#embed "../../secret-assets/textures/Display_Emissive.png"
};


constexpr const std::uint8_t DeatilsBG_Metallic[] = {
#embed "../../secret-assets/textures/DeatilsBG_Metallic.png"
};


constexpr const std::uint8_t Display_Metallic[] = {
#embed "../../secret-assets/textures/Display_Metallic.png"
};


constexpr const std::uint8_t MetalPlate02_Height[] = {
#embed "../../secret-assets/textures/MetalPlate02_Height.png"
};


constexpr const std::uint8_t MetalPlate01_Metallic[] = {
#embed "../../secret-assets/textures/MetalPlate01_Metallic.png"
};


constexpr const std::uint8_t Pipes01_Height[] = {
#embed "../../secret-assets/textures/Pipes01_Height.png"
};


constexpr const std::uint8_t MetalPlate03_Metallic[] = {
#embed "../../secret-assets/textures/MetalPlate03_Metallic.png"
};


constexpr const std::uint8_t Light01Blue_BaseColor[] = {
#embed "../../secret-assets/textures/Light01Blue_BaseColor.png"
};


constexpr const std::uint8_t Door03_Height[] = {
#embed "../../secret-assets/textures/Door03_Height.png"
};


constexpr const std::uint8_t DeatilsBG_Roughness[] = {
#embed "../../secret-assets/textures/DeatilsBG_Roughness.png"
};


constexpr const std::uint8_t Glass01_Height[] = {
#embed "../../secret-assets/textures/Glass01_Height.png"
};


constexpr const std::uint8_t Generator_BaseColor[] = {
#embed "../../secret-assets/textures/Generator_BaseColor.png"
};


constexpr const std::uint8_t SolarCell_BaseColor[] = {
#embed "../../secret-assets/textures/SolarCell_BaseColor.png"
};


constexpr const std::uint8_t Door03_Emissive[] = {
#embed "../../secret-assets/textures/Door03_Emissive.png"
};


constexpr const std::uint8_t Details01_Normal[] = {
#embed "../../secret-assets/textures/Details01_Normal.png"
};


constexpr const std::uint8_t Display_Roughness[] = {
#embed "../../secret-assets/textures/Display_Roughness.png"
};


constexpr const std::uint8_t Pipes01_Metallic[] = {
#embed "../../secret-assets/textures/Pipes01_Metallic.png"
};


constexpr const std::uint8_t Panel14Details_Roughness[] = {
#embed "../../secret-assets/textures/Panel14Details_Roughness.png"
};


constexpr const std::uint8_t Light02Red_Emissive[] = {
#embed "../../secret-assets/textures/Light02Red_Emissive.png"
};


constexpr const std::uint8_t Light01Blue_Height[] = {
#embed "../../secret-assets/textures/Light01Blue_Height.png"
};


constexpr const std::uint8_t Light02Red_BaseColor[] = {
#embed "../../secret-assets/textures/Light02Red_BaseColor.png"
};


constexpr const std::uint8_t MetalPlate01_BaseColor[] = {
#embed "../../secret-assets/textures/MetalPlate01_BaseColor.png"
};


constexpr const std::uint8_t T_Station01_Roughness[] = {
#embed "../../secret-assets/textures/T_Station01_Roughness.png"
};


constexpr const std::uint8_t Light01Blue_Normal[] = {
#embed "../../secret-assets/textures/Light01Blue_Normal.png"
};


constexpr const std::uint8_t Details01_AO[] = {
#embed "../../secret-assets/textures/Details01_AO.png"
};


constexpr const std::uint8_t Door01_Metallic[] = {
#embed "../../secret-assets/textures/Door01_Metallic.png"
};


constexpr const std::uint8_t MetalPlate03_Normal[] = {
#embed "../../secret-assets/textures/MetalPlate03_Normal.png"
};


constexpr const std::uint8_t Generator_Normal[] = {
#embed "../../secret-assets/textures/Generator_Normal.png"
};


constexpr const std::uint8_t Detail02_Height[] = {
#embed "../../secret-assets/textures/Detail02_Height.png"
};


constexpr const std::uint8_t Door02_Emissive[] = {
#embed "../../secret-assets/textures/Door02_Emissive.png"
};


constexpr const std::uint8_t Elevator_Emissive[] = {
#embed "../../secret-assets/textures/Elevator_Emissive.png"
};


constexpr const std::uint8_t SolarCell_Metallic[] = {
#embed "../../secret-assets/textures/SolarCell_Metallic.png"
};


constexpr const std::uint8_t Door01_Height[] = {
#embed "../../secret-assets/textures/Door01_Height.png"
};


constexpr const std::uint8_t Elevator_BaseColor[] = {
#embed "../../secret-assets/textures/Elevator_BaseColor.png"
};


constexpr const std::uint8_t Door02_Roughness[] = {
#embed "../../secret-assets/textures/Door02_Roughness.png"
};


constexpr const std::uint8_t Detail02_Metallic[] = {
#embed "../../secret-assets/textures/Detail02_Metallic.png"
};


constexpr const std::uint8_t Door02_Metallic[] = {
#embed "../../secret-assets/textures/Door02_Metallic.png"
};


constexpr const std::uint8_t Light01Blue_Emissive[] = {
#embed "../../secret-assets/textures/Light01Blue_Emissive.png"
};


constexpr const std::uint8_t DeatilsBG_Height[] = {
#embed "../../secret-assets/textures/DeatilsBG_Height.png"
};


constexpr const std::uint8_t T_Light_BC[] = {
#embed "../../secret-assets/textures/T_Light_BC.png"
};


constexpr const std::uint8_t T_Light_N[] = {
#embed "../../secret-assets/textures/T_Light_N.png"
};


constexpr const std::uint8_t T_Station01_BC[] = {
#embed "../../secret-assets/textures/T_Station01_BC.png"
};


constexpr const std::uint8_t Panel14Details_Normal[] = {
#embed "../../secret-assets/textures/Panel14Details_Normal.png"
};


constexpr const std::uint8_t Pipes01_BaseColor[] = {
#embed "../../secret-assets/textures/Pipes01_BaseColor.png"
};


constexpr const std::uint8_t Details03_BaseColor[] = {
#embed "../../secret-assets/textures/Details03_BaseColor.png"
};


constexpr const std::uint8_t Details01_Emissive[] = {
#embed "../../secret-assets/textures/Details01_Emissive.png"
};


constexpr const std::uint8_t Details03_Height[] = {
#embed "../../secret-assets/textures/Details03_Height.png"
};


constexpr const std::uint8_t Pipes01_Normal[] = {
#embed "../../secret-assets/textures/Pipes01_Normal.png"
};


constexpr const std::uint8_t Detail02_Emissive[] = {
#embed "../../secret-assets/textures/Detail02_Emissive.png"
};


constexpr const std::uint8_t SolarCell_Roughness[] = {
#embed "../../secret-assets/textures/SolarCell_Roughness.png"
};


constexpr const std::uint8_t SM_Sign_A[] = {
#embed "../../secret-assets/models/SM_Sign_A.fbx"
};


constexpr const std::uint8_t SM_Corridor033_12_06_Bottom_B[] = {
#embed "../../secret-assets/models/SM_Corridor033_12_06_Bottom_B.fbx"
};


constexpr const std::uint8_t SM_Details31[] = {
#embed "../../secret-assets/models/SM_Details31.fbx"
};


constexpr const std::uint8_t SM_Station[] = {
#embed "../../secret-assets/models/SM_Station.fbx"
};


constexpr const std::uint8_t SM_Corridor019_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor019_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Corner05_8_8_A_L[] = {
#embed "../../secret-assets/models/SM_Corner05_8_8_A_L.fbx"
};


constexpr const std::uint8_t SM_Panel13[] = {
#embed "../../secret-assets/models/SM_Panel13.fbx"
};


constexpr const std::uint8_t SM_Sign_2[] = {
#embed "../../secret-assets/models/SM_Sign_2.fbx"
};


constexpr const std::uint8_t SM_Sign_V[] = {
#embed "../../secret-assets/models/SM_Sign_V.fbx"
};


constexpr const std::uint8_t SM_Corridor043_12_06_Bottom_B[] = {
#embed "../../secret-assets/models/SM_Corridor043_12_06_Bottom_B.fbx"
};


constexpr const std::uint8_t SM_Corridor050_02_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor050_02_04_A.fbx"
};


constexpr const std::uint8_t SM_Sign_W[] = {
#embed "../../secret-assets/models/SM_Sign_W.fbx"
};


constexpr const std::uint8_t SM_Light013[] = {
#embed "../../secret-assets/models/SM_Light013.fbx"
};


constexpr const std::uint8_t SM_Door03FramLock02[] = {
#embed "../../secret-assets/models/SM_Door03FramLock02.fbx"
};


constexpr const std::uint8_t SM_Axis01[] = {
#embed "../../secret-assets/models/SM_Axis01.fbx"
};


constexpr const std::uint8_t SM_ElevatorEnd[] = {
#embed "../../secret-assets/models/SM_ElevatorEnd.fbx"
};


constexpr const std::uint8_t SM_Sign_N[] = {
#embed "../../secret-assets/models/SM_Sign_N.fbx"
};


constexpr const std::uint8_t SM_Detail16[] = {
#embed "../../secret-assets/models/SM_Detail16.fbx"
};


constexpr const std::uint8_t SM_Sign_ElevatorLight[] = {
#embed "../../secret-assets/models/SM_Sign_ElevatorLight.fbx"
};


constexpr const std::uint8_t SM_Monitor[] = {
#embed "../../secret-assets/models/SM_Monitor.fbx"
};


constexpr const std::uint8_t SM_Door03FramLock04[] = {
#embed "../../secret-assets/models/SM_Door03FramLock04.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd11_E[] = {
#embed "../../secret-assets/models/SM_CorridorEnd11_E.fbx"
};


constexpr const std::uint8_t SM_FloorDoor[] = {
#embed "../../secret-assets/models/SM_FloorDoor.fbx"
};


constexpr const std::uint8_t SM_Generator_Wheel03[] = {
#embed "../../secret-assets/models/SM_Generator_Wheel03.fbx"
};


constexpr const std::uint8_t SM_Sign_Cargo[] = {
#embed "../../secret-assets/models/SM_Sign_Cargo.fbx"
};


constexpr const std::uint8_t SM_Corridor015_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor015_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Panel02[] = {
#embed "../../secret-assets/models/SM_Panel02.fbx"
};


constexpr const std::uint8_t SM_Door03B_Light[] = {
#embed "../../secret-assets/models/SM_Door03B_Light.fbx"
};


constexpr const std::uint8_t SM_Sign_DangerPicture1[] = {
#embed "../../secret-assets/models/SM_Sign_DangerPicture1.fbx"
};


constexpr const std::uint8_t SM_panel03[] = {
#embed "../../secret-assets/models/SM_panel03.fbx"
};


constexpr const std::uint8_t SM_FloorPanel08_02_02[] = {
#embed "../../secret-assets/models/SM_FloorPanel08_02_02.fbx"
};


constexpr const std::uint8_t SM_Windows01_12_8[] = {
#embed "../../secret-assets/models/SM_Windows01_12_8.fbx"
};


constexpr const std::uint8_t SM_CelilingPanel03_03_04[] = {
#embed "../../secret-assets/models/SM_CelilingPanel03_03_04.fbx"
};


constexpr const std::uint8_t SM_Stairs01_02_04[] = {
#embed "../../secret-assets/models/SM_Stairs01_02_04.fbx"
};


constexpr const std::uint8_t SM_Light005[] = {
#embed "../../secret-assets/models/SM_Light005.fbx"
};


constexpr const std::uint8_t SM_ElevatorTunnelNoLevel[] = {
#embed "../../secret-assets/models/SM_ElevatorTunnelNoLevel.fbx"
};


constexpr const std::uint8_t SM_ServiceTuinnelDoor01[] = {
#embed "../../secret-assets/models/SM_ServiceTuinnelDoor01.fbx"
};


constexpr const std::uint8_t SM_Sign_C[] = {
#embed "../../secret-assets/models/SM_Sign_C.fbx"
};


constexpr const std::uint8_t SM_FloorPanel03_02_02[] = {
#embed "../../secret-assets/models/SM_FloorPanel03_02_02.fbx"
};


constexpr const std::uint8_t SM_Sign_DangerPicture2[] = {
#embed "../../secret-assets/models/SM_Sign_DangerPicture2.fbx"
};


constexpr const std::uint8_t SM_Door01Frame[] = {
#embed "../../secret-assets/models/SM_Door01Frame.fbx"
};


constexpr const std::uint8_t SM_Detail13[] = {
#embed "../../secret-assets/models/SM_Detail13.fbx"
};


constexpr const std::uint8_t SM_Sign_Danger[] = {
#embed "../../secret-assets/models/SM_Sign_Danger.fbx"
};


constexpr const std::uint8_t SM_Panel07[] = {
#embed "../../secret-assets/models/SM_Panel07.fbx"
};


constexpr const std::uint8_t SM_Windows02_12_6[] = {
#embed "../../secret-assets/models/SM_Windows02_12_6.fbx"
};


constexpr const std::uint8_t SM_CeilingDoor[] = {
#embed "../../secret-assets/models/SM_CeilingDoor.fbx"
};


constexpr const std::uint8_t SM_Sign_Board[] = {
#embed "../../secret-assets/models/SM_Sign_Board.fbx"
};


constexpr const std::uint8_t SM_Corridor040_12_06_Top_B_00[] = {
#embed "../../secret-assets/models/SM_Corridor040_12_06_Top_B_00.fbx"
};


constexpr const std::uint8_t SM_Fan02[] = {
#embed "../../secret-assets/models/SM_Fan02.fbx"
};


constexpr const std::uint8_t SM_Fan03[] = {
#embed "../../secret-assets/models/SM_Fan03.fbx"
};


constexpr const std::uint8_t SM_SignRoom[] = {
#embed "../../secret-assets/models/SM_SignRoom.fbx"
};


constexpr const std::uint8_t SM_Sign_Level[] = {
#embed "../../secret-assets/models/SM_Sign_Level.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd02_A[] = {
#embed "../../secret-assets/models/SM_CorridorEnd02_A.fbx"
};


constexpr const std::uint8_t SM_Sign_D[] = {
#embed "../../secret-assets/models/SM_Sign_D.fbx"
};


constexpr const std::uint8_t SM_SignDanger1[] = {
#embed "../../secret-assets/models/SM_SignDanger1.fbx"
};


constexpr const std::uint8_t SM_Sign_Elevator[] = {
#embed "../../secret-assets/models/SM_Sign_Elevator.fbx"
};


constexpr const std::uint8_t SM_FloorPanel16_04_02[] = {
#embed "../../secret-assets/models/SM_FloorPanel16_04_02.fbx"
};


constexpr const std::uint8_t SM_Sign_Deck[] = {
#embed "../../secret-assets/models/SM_Sign_Deck.fbx"
};


constexpr const std::uint8_t SM_Detail07[] = {
#embed "../../secret-assets/models/SM_Detail07.fbx"
};


constexpr const std::uint8_t SM_Sign_H[] = {
#embed "../../secret-assets/models/SM_Sign_H.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd06_D[] = {
#embed "../../secret-assets/models/SM_CorridorEnd06_D.fbx"
};


constexpr const std::uint8_t SM_Detail10[] = {
#embed "../../secret-assets/models/SM_Detail10.fbx"
};


constexpr const std::uint8_t SM_Gate01_A_B[] = {
#embed "../../secret-assets/models/SM_Gate01_A_B.fbx"
};


constexpr const std::uint8_t SM_Door03B[] = {
#embed "../../secret-assets/models/SM_Door03B.fbx"
};


constexpr const std::uint8_t SM_Light007[] = {
#embed "../../secret-assets/models/SM_Light007.fbx"
};


constexpr const std::uint8_t SM_Details35[] = {
#embed "../../secret-assets/models/SM_Details35.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel05_04_HV[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel05_04_HV.fbx"
};


constexpr const std::uint8_t SM_Corridor037_12_06_Top_B_04[] = {
#embed "../../secret-assets/models/SM_Corridor037_12_06_Top_B_04.fbx"
};


constexpr const std::uint8_t SM_Corridor008_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor008_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Light011[] = {
#embed "../../secret-assets/models/SM_Light011.fbx"
};


constexpr const std::uint8_t SM_Details33[] = {
#embed "../../secret-assets/models/SM_Details33.fbx"
};


constexpr const std::uint8_t SM_Sign_K[] = {
#embed "../../secret-assets/models/SM_Sign_K.fbx"
};


constexpr const std::uint8_t SM_Corridor021_08_02_Top_D_03[] = {
#embed "../../secret-assets/models/SM_Corridor021_08_02_Top_D_03.fbx"
};


constexpr const std::uint8_t SM_Panel10[] = {
#embed "../../secret-assets/models/SM_Panel10.fbx"
};


constexpr const std::uint8_t SM_Sign_E[] = {
#embed "../../secret-assets/models/SM_Sign_E.fbx"
};


constexpr const std::uint8_t SM_Corridor041_12_06_Top_B_04[] = {
#embed "../../secret-assets/models/SM_Corridor041_12_06_Top_B_04.fbx"
};


constexpr const std::uint8_t SM_Corridor017_04_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor017_04_04_A.fbx"
};


constexpr const std::uint8_t SM_Sign_Q[] = {
#embed "../../secret-assets/models/SM_Sign_Q.fbx"
};


constexpr const std::uint8_t SM_FloorPanel05_02_04[] = {
#embed "../../secret-assets/models/SM_FloorPanel05_02_04.fbx"
};


constexpr const std::uint8_t SM_CelilingPanel04_03_04[] = {
#embed "../../secret-assets/models/SM_CelilingPanel04_03_04.fbx"
};


constexpr const std::uint8_t SM_Sign_6[] = {
#embed "../../secret-assets/models/SM_Sign_6.fbx"
};


constexpr const std::uint8_t SM_SignCaution1[] = {
#embed "../../secret-assets/models/SM_SignCaution1.fbx"
};


constexpr const std::uint8_t SM_Sign_3[] = {
#embed "../../secret-assets/models/SM_Sign_3.fbx"
};


constexpr const std::uint8_t SM_Generator_Wheel01[] = {
#embed "../../secret-assets/models/SM_Generator_Wheel01.fbx"
};


constexpr const std::uint8_t SM_Corridor046_06_08_ServiceTunnelDoor[] = {
#embed "../../secret-assets/models/SM_Corridor046_06_08_ServiceTunnelDoor.fbx"
};


constexpr const std::uint8_t SM_Corridor046_04_04[] = {
#embed "../../secret-assets/models/SM_Corridor046_04_04.fbx"
};


constexpr const std::uint8_t SM_Corridor014_04_04[] = {
#embed "../../secret-assets/models/SM_Corridor014_04_04.fbx"
};


constexpr const std::uint8_t SM_Corner01_8_8_X[] = {
#embed "../../secret-assets/models/SM_Corner01_8_8_X.fbx"
};


constexpr const std::uint8_t SM_Sign_O[] = {
#embed "../../secret-assets/models/SM_Sign_O.fbx"
};


constexpr const std::uint8_t SM_ElevatorDoor04[] = {
#embed "../../secret-assets/models/SM_ElevatorDoor04.fbx"
};


constexpr const std::uint8_t SM_Windows04_8_6[] = {
#embed "../../secret-assets/models/SM_Windows04_8_6.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd04_B[] = {
#embed "../../secret-assets/models/SM_CorridorEnd04_B.fbx"
};


constexpr const std::uint8_t SM_Closer03[] = {
#embed "../../secret-assets/models/SM_Closer03.fbx"
};


constexpr const std::uint8_t SM_Detail03[] = {
#embed "../../secret-assets/models/SM_Detail03.fbx"
};


constexpr const std::uint8_t SM_Panel14UE[] = {
#embed "../../secret-assets/models/SM_Panel14UE.fbx"
};


constexpr const std::uint8_t SM_Sign_Port[] = {
#embed "../../secret-assets/models/SM_Sign_Port.fbx"
};


constexpr const std::uint8_t SM_Details26[] = {
#embed "../../secret-assets/models/SM_Details26.fbx"
};


constexpr const std::uint8_t SM_Elevator[] = {
#embed "../../secret-assets/models/SM_Elevator.fbx"
};


constexpr const std::uint8_t SM_FloorPanel14_02_01[] = {
#embed "../../secret-assets/models/SM_FloorPanel14_02_01.fbx"
};


constexpr const std::uint8_t SM_Windows03_12_4[] = {
#embed "../../secret-assets/models/SM_Windows03_12_4.fbx"
};


constexpr const std::uint8_t SM_SignBase002[] = {
#embed "../../secret-assets/models/SM_SignBase002.fbx"
};


constexpr const std::uint8_t SM_Gear02[] = {
#embed "../../secret-assets/models/SM_Gear02.fbx"
};


constexpr const std::uint8_t SM_FloorPanel12_01_01[] = {
#embed "../../secret-assets/models/SM_FloorPanel12_01_01.fbx"
};


constexpr const std::uint8_t SM_Generator[] = {
#embed "../../secret-assets/models/SM_Generator.fbx"
};


constexpr const std::uint8_t SM_Light001[] = {
#embed "../../secret-assets/models/SM_Light001.fbx"
};


constexpr const std::uint8_t SM_Sign_S[] = {
#embed "../../secret-assets/models/SM_Sign_S.fbx"
};


constexpr const std::uint8_t SM_Panel08[] = {
#embed "../../secret-assets/models/SM_Panel08.fbx"
};


constexpr const std::uint8_t SM_Details30[] = {
#embed "../../secret-assets/models/SM_Details30.fbx"
};


constexpr const std::uint8_t SM_Corner02_8_8_A_T[] = {
#embed "../../secret-assets/models/SM_Corner02_8_8_A_T.fbx"
};


constexpr const std::uint8_t SM_Door03A_Lock[] = {
#embed "../../secret-assets/models/SM_Door03A_Lock.fbx"
};


constexpr const std::uint8_t SM_Corridor009_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor009_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Corridor053_08_08_A[] = {
#embed "../../secret-assets/models/SM_Corridor053_08_08_A.fbx"
};


constexpr const std::uint8_t SM_Details27[] = {
#embed "../../secret-assets/models/SM_Details27.fbx"
};


constexpr const std::uint8_t SM_Sign_G[] = {
#embed "../../secret-assets/models/SM_Sign_G.fbx"
};


constexpr const std::uint8_t SM_Corridor029_08_04_Top_D_01[] = {
#embed "../../secret-assets/models/SM_Corridor029_08_04_Top_D_01.fbx"
};


constexpr const std::uint8_t SM_FloorPanel10_07_04[] = {
#embed "../../secret-assets/models/SM_FloorPanel10_07_04.fbx"
};


constexpr const std::uint8_t SM_Light002[] = {
#embed "../../secret-assets/models/SM_Light002.fbx"
};


constexpr const std::uint8_t SM_Sign_EngineRoom[] = {
#embed "../../secret-assets/models/SM_Sign_EngineRoom.fbx"
};


constexpr const std::uint8_t SM_Door03A_Light[] = {
#embed "../../secret-assets/models/SM_Door03A_Light.fbx"
};


constexpr const std::uint8_t SM_Sign_Caution[] = {
#embed "../../secret-assets/models/SM_Sign_Caution.fbx"
};


constexpr const std::uint8_t SM_Corridor031_08_04_Bottom_D[] = {
#embed "../../secret-assets/models/SM_Corridor031_08_04_Bottom_D.fbx"
};


constexpr const std::uint8_t SM_Sign_F[] = {
#embed "../../secret-assets/models/SM_Sign_F.fbx"
};


constexpr const std::uint8_t SM_Corridor023_08_04_Bottom_D[] = {
#embed "../../secret-assets/models/SM_Corridor023_08_04_Bottom_D.fbx"
};


constexpr const std::uint8_t SM_Corridor039_12_06_Top_B_02[] = {
#embed "../../secret-assets/models/SM_Corridor039_12_06_Top_B_02.fbx"
};


constexpr const std::uint8_t SM_Details21[] = {
#embed "../../secret-assets/models/SM_Details21.fbx"
};


constexpr const std::uint8_t SM_Corridor051_02_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor051_02_04_A.fbx"
};


constexpr const std::uint8_t SM_Light009Light[] = {
#embed "../../secret-assets/models/SM_Light009Light.fbx"
};


constexpr const std::uint8_t SM_Corridor028_08_04_Bottom_D[] = {
#embed "../../secret-assets/models/SM_Corridor028_08_04_Bottom_D.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd05_D[] = {
#embed "../../secret-assets/models/SM_CorridorEnd05_D.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd07_D[] = {
#embed "../../secret-assets/models/SM_CorridorEnd07_D.fbx"
};


constexpr const std::uint8_t SM_Sign_9[] = {
#embed "../../secret-assets/models/SM_Sign_9.fbx"
};


constexpr const std::uint8_t SM_Detail05[] = {
#embed "../../secret-assets/models/SM_Detail05.fbx"
};


constexpr const std::uint8_t SM_CelilingPanel01_04_02[] = {
#embed "../../secret-assets/models/SM_CelilingPanel01_04_02.fbx"
};


constexpr const std::uint8_t SM_Sign_M[] = {
#embed "../../secret-assets/models/SM_Sign_M.fbx"
};


constexpr const std::uint8_t SM_Sign_P[] = {
#embed "../../secret-assets/models/SM_Sign_P.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel01_04_H[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel01_04_H.fbx"
};


constexpr const std::uint8_t SM_FloorPanel11_03_03[] = {
#embed "../../secret-assets/models/SM_FloorPanel11_03_03.fbx"
};


constexpr const std::uint8_t SM_Corridor018_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor018_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Light006[] = {
#embed "../../secret-assets/models/SM_Light006.fbx"
};


constexpr const std::uint8_t SM_Axis02[] = {
#embed "../../secret-assets/models/SM_Axis02.fbx"
};


constexpr const std::uint8_t SM_Sign_Gate[] = {
#embed "../../secret-assets/models/SM_Sign_Gate.fbx"
};


constexpr const std::uint8_t SM_Details24[] = {
#embed "../../secret-assets/models/SM_Details24.fbx"
};


constexpr const std::uint8_t SM_Corridor020_08_02_Bottom_D[] = {
#embed "../../secret-assets/models/SM_Corridor020_08_02_Bottom_D.fbx"
};


constexpr const std::uint8_t SM_Sign_RightArrow[] = {
#embed "../../secret-assets/models/SM_Sign_RightArrow.fbx"
};


constexpr const std::uint8_t SM_Windows05_6_4[] = {
#embed "../../secret-assets/models/SM_Windows05_6_4.fbx"
};


constexpr const std::uint8_t SM_Corner03_12_12_B_X[] = {
#embed "../../secret-assets/models/SM_Corner03_12_12_B_X.fbx"
};


constexpr const std::uint8_t SM_Corridor024_08_04_Top_D_01[] = {
#embed "../../secret-assets/models/SM_Corridor024_08_04_Top_D_01.fbx"
};


constexpr const std::uint8_t SM_Corridor010_02_02[] = {
#embed "../../secret-assets/models/SM_Corridor010_02_02.fbx"
};


constexpr const std::uint8_t SM_Stairs03_02_04[] = {
#embed "../../secret-assets/models/SM_Stairs03_02_04.fbx"
};


constexpr const std::uint8_t SM_Details32[] = {
#embed "../../secret-assets/models/SM_Details32.fbx"
};


constexpr const std::uint8_t SM_Sign_TechnicalRoom[] = {
#embed "../../secret-assets/models/SM_Sign_TechnicalRoom.fbx"
};


constexpr const std::uint8_t SM_ElevatorDoor01[] = {
#embed "../../secret-assets/models/SM_ElevatorDoor01.fbx"
};


constexpr const std::uint8_t SM_Corridor_Stairs01_04_02[] = {
#embed "../../secret-assets/models/SM_Corridor_Stairs01_04_02.fbx"
};


constexpr const std::uint8_t SM_Corridor052_02_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor052_02_04_A.fbx"
};


constexpr const std::uint8_t SM_Door03FramLock01[] = {
#embed "../../secret-assets/models/SM_Door03FramLock01.fbx"
};


constexpr const std::uint8_t SM_ElevatorCabin[] = {
#embed "../../secret-assets/models/SM_ElevatorCabin.fbx"
};


constexpr const std::uint8_t SM_Details38[] = {
#embed "../../secret-assets/models/SM_Details38.fbx"
};


constexpr const std::uint8_t SM_Corridor003_04_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor003_04_04_A.fbx"
};


constexpr const std::uint8_t SM_Door03B_Lock[] = {
#embed "../../secret-assets/models/SM_Door03B_Lock.fbx"
};


constexpr const std::uint8_t SM_ElevatorDoor02[] = {
#embed "../../secret-assets/models/SM_ElevatorDoor02.fbx"
};


constexpr const std::uint8_t SM_Detail11[] = {
#embed "../../secret-assets/models/SM_Detail11.fbx"
};


constexpr const std::uint8_t SM_Sign_J[] = {
#embed "../../secret-assets/models/SM_Sign_J.fbx"
};


constexpr const std::uint8_t SM_Details22[] = {
#embed "../../secret-assets/models/SM_Details22.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd09_E[] = {
#embed "../../secret-assets/models/SM_CorridorEnd09_E.fbx"
};


constexpr const std::uint8_t SM_Panel09[] = {
#embed "../../secret-assets/models/SM_Panel09.fbx"
};


constexpr const std::uint8_t SM_Sign_Prior[] = {
#embed "../../secret-assets/models/SM_Sign_Prior.fbx"
};


constexpr const std::uint8_t SM_Sign_7[] = {
#embed "../../secret-assets/models/SM_Sign_7.fbx"
};


constexpr const std::uint8_t SM_Corridor042_12_06_Bottom_B[] = {
#embed "../../secret-assets/models/SM_Corridor042_12_06_Bottom_B.fbx"
};


constexpr const std::uint8_t SM_Door02_LOCK_B[] = {
#embed "../../secret-assets/models/SM_Door02_LOCK_B.fbx"
};


constexpr const std::uint8_t SM_Details34[] = {
#embed "../../secret-assets/models/SM_Details34.fbx"
};


constexpr const std::uint8_t SM_Details19[] = {
#embed "../../secret-assets/models/SM_Details19.fbx"
};


constexpr const std::uint8_t SM_Corridor049_08_04_ServiceTunnelDoor[] = {
#embed "../../secret-assets/models/SM_Corridor049_08_04_ServiceTunnelDoor.fbx"
};


constexpr const std::uint8_t SM_Detail06[] = {
#embed "../../secret-assets/models/SM_Detail06.fbx"
};


constexpr const std::uint8_t SM_Corridor012_02_02[] = {
#embed "../../secret-assets/models/SM_Corridor012_02_02.fbx"
};


constexpr const std::uint8_t SM_Corridor025_08_04_Top_D_03[] = {
#embed "../../secret-assets/models/SM_Corridor025_08_04_Top_D_03.fbx"
};


constexpr const std::uint8_t SM_Detail12[] = {
#embed "../../secret-assets/models/SM_Detail12.fbx"
};


constexpr const std::uint8_t SM_ServiceTuinnelDoor02[] = {
#embed "../../secret-assets/models/SM_ServiceTuinnelDoor02.fbx"
};


constexpr const std::uint8_t SM_Light010[] = {
#embed "../../secret-assets/models/SM_Light010.fbx"
};


constexpr const std::uint8_t SM_Sign_MedicalRoom[] = {
#embed "../../secret-assets/models/SM_Sign_MedicalRoom.fbx"
};


constexpr const std::uint8_t SM_Detail15[] = {
#embed "../../secret-assets/models/SM_Detail15.fbx"
};


constexpr const std::uint8_t SM_Gear01[] = {
#embed "../../secret-assets/models/SM_Gear01.fbx"
};


constexpr const std::uint8_t SM_Door01B01[] = {
#embed "../../secret-assets/models/SM_Door01B01.fbx"
};


constexpr const std::uint8_t SM_FloorPanel01_01_01[] = {
#embed "../../secret-assets/models/SM_FloorPanel01_01_01.fbx"
};


constexpr const std::uint8_t SM_Details25[] = {
#embed "../../secret-assets/models/SM_Details25.fbx"
};


constexpr const std::uint8_t SM_Light003[] = {
#embed "../../secret-assets/models/SM_Light003.fbx"
};


constexpr const std::uint8_t SM_Light008[] = {
#embed "../../secret-assets/models/SM_Light008.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd08_E[] = {
#embed "../../secret-assets/models/SM_CorridorEnd08_E.fbx"
};


constexpr const std::uint8_t SM_FloorPanel15_03_03[] = {
#embed "../../secret-assets/models/SM_FloorPanel15_03_03.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel08_04_VH[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel08_04_VH.fbx"
};


constexpr const std::uint8_t SM_Detail17[] = {
#embed "../../secret-assets/models/SM_Detail17.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel06_04_V[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel06_04_V.fbx"
};


constexpr const std::uint8_t SM_Details28[] = {
#embed "../../secret-assets/models/SM_Details28.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel04_04_HC[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel04_04_HC.fbx"
};


constexpr const std::uint8_t SM_Corridor011_02_03[] = {
#embed "../../secret-assets/models/SM_Corridor011_02_03.fbx"
};


constexpr const std::uint8_t SM_Corridor005_04_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor005_04_04_A.fbx"
};


constexpr const std::uint8_t SM_Corridor007_06_08_B[] = {
#embed "../../secret-assets/models/SM_Corridor007_06_08_B.fbx"
};


constexpr const std::uint8_t SM_Door01A01[] = {
#embed "../../secret-assets/models/SM_Door01A01.fbx"
};


constexpr const std::uint8_t SM_Corridor044_12_Closer_B[] = {
#embed "../../secret-assets/models/SM_Corridor044_12_Closer_B.fbx"
};


constexpr const std::uint8_t SM_Closer02[] = {
#embed "../../secret-assets/models/SM_Closer02.fbx"
};


constexpr const std::uint8_t SM_Light009[] = {
#embed "../../secret-assets/models/SM_Light009.fbx"
};


constexpr const std::uint8_t SM_SignHighVoltage1[] = {
#embed "../../secret-assets/models/SM_SignHighVoltage1.fbx"
};


constexpr const std::uint8_t SM_Sign__[] = {
#embed "../../secret-assets/models/SM_Sign__.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd01_A[] = {
#embed "../../secret-assets/models/SM_CorridorEnd01_A.fbx"
};


constexpr const std::uint8_t SM_Panel05[] = {
#embed "../../secret-assets/models/SM_Panel05.fbx"
};


constexpr const std::uint8_t SM_Corridor035_12_06_Top_B_00[] = {
#embed "../../secret-assets/models/SM_Corridor035_12_06_Top_B_00.fbx"
};


constexpr const std::uint8_t SM_Door01B02[] = {
#embed "../../secret-assets/models/SM_Door01B02.fbx"
};


constexpr const std::uint8_t SM_Door02_Light01[] = {
#embed "../../secret-assets/models/SM_Door02_Light01.fbx"
};


constexpr const std::uint8_t SM_CelilingPanel02_04_04[] = {
#embed "../../secret-assets/models/SM_CelilingPanel02_04_04.fbx"
};


constexpr const std::uint8_t SM_Sign_8[] = {
#embed "../../secret-assets/models/SM_Sign_8.fbx"
};


constexpr const std::uint8_t SM_Corridor030_08_04_Top_D_03[] = {
#embed "../../secret-assets/models/SM_Corridor030_08_04_Top_D_03.fbx"
};


constexpr const std::uint8_t SM_Details37[] = {
#embed "../../secret-assets/models/SM_Details37.fbx"
};


constexpr const std::uint8_t SM_Sign_Sector[] = {
#embed "../../secret-assets/models/SM_Sign_Sector.fbx"
};


constexpr const std::uint8_t SM_Corridor036_12_06_Top_B_02[] = {
#embed "../../secret-assets/models/SM_Corridor036_12_06_Top_B_02.fbx"
};


constexpr const std::uint8_t SM_Sign_5[] = {
#embed "../../secret-assets/models/SM_Sign_5.fbx"
};


constexpr const std::uint8_t SM_Corridor004_04_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor004_04_04_A.fbx"
};


constexpr const std::uint8_t SM_Details36[] = {
#embed "../../secret-assets/models/SM_Details36.fbx"
};


constexpr const std::uint8_t SM_Stairs02_02_02[] = {
#embed "../../secret-assets/models/SM_Stairs02_02_02.fbx"
};


constexpr const std::uint8_t SM_Details29[] = {
#embed "../../secret-assets/models/SM_Details29.fbx"
};


constexpr const std::uint8_t SM_Door03A[] = {
#embed "../../secret-assets/models/SM_Door03A.fbx"
};


constexpr const std::uint8_t SM_FloorPanel06_005_02[] = {
#embed "../../secret-assets/models/SM_FloorPanel06_005_02.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd03_B[] = {
#embed "../../secret-assets/models/SM_CorridorEnd03_B.fbx"
};


constexpr const std::uint8_t SM_Panel12[] = {
#embed "../../secret-assets/models/SM_Panel12.fbx"
};


constexpr const std::uint8_t SM_Corridor022_08_02_Top_D_01[] = {
#embed "../../secret-assets/models/SM_Corridor022_08_02_Top_D_01.fbx"
};


constexpr const std::uint8_t SM_ElevatorDoor03[] = {
#embed "../../secret-assets/models/SM_ElevatorDoor03.fbx"
};


constexpr const std::uint8_t SM_Corridor046_06_08_Light[] = {
#embed "../../secret-assets/models/SM_Corridor046_06_08_Light.fbx"
};


constexpr const std::uint8_t SM_CorridorEnd10_E[] = {
#embed "../../secret-assets/models/SM_CorridorEnd10_E.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel03_04_H[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel03_04_H.fbx"
};


constexpr const std::uint8_t SM_Corridor006_04_08_A[] = {
#embed "../../secret-assets/models/SM_Corridor006_04_08_A.fbx"
};


constexpr const std::uint8_t SM_Light012[] = {
#embed "../../secret-assets/models/SM_Light012.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel07_04_V[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel07_04_V.fbx"
};


constexpr const std::uint8_t SM_Door02_LOCK_A[] = {
#embed "../../secret-assets/models/SM_Door02_LOCK_A.fbx"
};


constexpr const std::uint8_t SM_Panel11[] = {
#embed "../../secret-assets/models/SM_Panel11.fbx"
};


constexpr const std::uint8_t SM_Detail08[] = {
#embed "../../secret-assets/models/SM_Detail08.fbx"
};


constexpr const std::uint8_t SM_Corridor013_02_02[] = {
#embed "../../secret-assets/models/SM_Corridor013_02_02.fbx"
};


constexpr const std::uint8_t Panel14Light[] = {
#embed "../../secret-assets/models/Panel14Light.fbx"
};


constexpr const std::uint8_t SM_Detail04[] = {
#embed "../../secret-assets/models/SM_Detail04.fbx"
};


constexpr const std::uint8_t SM_FloorPanel13_04_01[] = {
#embed "../../secret-assets/models/SM_FloorPanel13_04_01.fbx"
};


constexpr const std::uint8_t SM_Gear03[] = {
#embed "../../secret-assets/models/SM_Gear03.fbx"
};


constexpr const std::uint8_t SM_SolarCell[] = {
#embed "../../secret-assets/models/SM_SolarCell.fbx"
};


constexpr const std::uint8_t SM_Planet[] = {
#embed "../../secret-assets/models/SM_Planet.fbx"
};


constexpr const std::uint8_t SM_Panel04[] = {
#embed "../../secret-assets/models/SM_Panel04.fbx"
};


constexpr const std::uint8_t SM_Gear04[] = {
#embed "../../secret-assets/models/SM_Gear04.fbx"
};


constexpr const std::uint8_t SM_Corridor002_04_02_A[] = {
#embed "../../secret-assets/models/SM_Corridor002_04_02_A.fbx"
};


constexpr const std::uint8_t SM_Corner06_12_12_B_L[] = {
#embed "../../secret-assets/models/SM_Corner06_12_12_B_L.fbx"
};


constexpr const std::uint8_t SM_Detail09[] = {
#embed "../../secret-assets/models/SM_Detail09.fbx"
};


constexpr const std::uint8_t SM_Sign_T[] = {
#embed "../../secret-assets/models/SM_Sign_T.fbx"
};


constexpr const std::uint8_t SM_Details23[] = {
#embed "../../secret-assets/models/SM_Details23.fbx"
};


constexpr const std::uint8_t SM_Door03FramLock03[] = {
#embed "../../secret-assets/models/SM_Door03FramLock03.fbx"
};


constexpr const std::uint8_t SM_Gate01_A_B_D[] = {
#embed "../../secret-assets/models/SM_Gate01_A_B_D.fbx"
};


constexpr const std::uint8_t SM_Sign_U[] = {
#embed "../../secret-assets/models/SM_Sign_U.fbx"
};


constexpr const std::uint8_t SM_Sign_Door[] = {
#embed "../../secret-assets/models/SM_Sign_Door.fbx"
};


constexpr const std::uint8_t SM_Details23Animated[] = {
#embed "../../secret-assets/models/SM_Details23Animated.fbx"
};


constexpr const std::uint8_t SM_Sign_Lock[] = {
#embed "../../secret-assets/models/SM_Sign_Lock.fbx"
};


constexpr const std::uint8_t SM_Light004[] = {
#embed "../../secret-assets/models/SM_Light004.fbx"
};


constexpr const std::uint8_t SM_Sign_MaintenenceRoom[] = {
#embed "../../secret-assets/models/SM_Sign_MaintenenceRoom.fbx"
};


constexpr const std::uint8_t SM_Sign_1[] = {
#embed "../../secret-assets/models/SM_Sign_1.fbx"
};


constexpr const std::uint8_t SM_Generator_Wheel02[] = {
#embed "../../secret-assets/models/SM_Generator_Wheel02.fbx"
};


constexpr const std::uint8_t SM_Sign_R[] = {
#embed "../../secret-assets/models/SM_Sign_R.fbx"
};


constexpr const std::uint8_t SM_Fan01[] = {
#embed "../../secret-assets/models/SM_Fan01.fbx"
};


constexpr const std::uint8_t SM_Corridor038_12_06_Bottom_B[] = {
#embed "../../secret-assets/models/SM_Corridor038_12_06_Bottom_B.fbx"
};


constexpr const std::uint8_t SM_Door01Frame01_A_B[] = {
#embed "../../secret-assets/models/SM_Door01Frame01_A_B.fbx"
};


constexpr const std::uint8_t SM_Corridor027_08_02_Top_D_01[] = {
#embed "../../secret-assets/models/SM_Corridor027_08_02_Top_D_01.fbx"
};


constexpr const std::uint8_t SM_Corridor045_04_04[] = {
#embed "../../secret-assets/models/SM_Corridor045_04_04.fbx"
};


constexpr const std::uint8_t SM_Panel06[] = {
#embed "../../secret-assets/models/SM_Panel06.fbx"
};


constexpr const std::uint8_t SM_Corridor016_04_04_A[] = {
#embed "../../secret-assets/models/SM_Corridor016_04_04_A.fbx"
};


constexpr const std::uint8_t SM_Sign_0[] = {
#embed "../../secret-assets/models/SM_Sign_0.fbx"
};


constexpr const std::uint8_t SM_Sign__LeftArrow[] = {
#embed "../../secret-assets/models/SM_Sign__LeftArrow.fbx"
};


constexpr const std::uint8_t SM_Corridor001_04_02_A[] = {
#embed "../../secret-assets/models/SM_Corridor001_04_02_A.fbx"
};


constexpr const std::uint8_t SM_Panel14[] = {
#embed "../../secret-assets/models/SM_Panel14.fbx"
};


constexpr const std::uint8_t SM_Panel01[] = {
#embed "../../secret-assets/models/SM_Panel01.fbx"
};


constexpr const std::uint8_t SM_FloorPanel04_005_02[] = {
#embed "../../secret-assets/models/SM_FloorPanel04_005_02.fbx"
};


constexpr const std::uint8_t SM_Detail01[] = {
#embed "../../secret-assets/models/SM_Detail01.fbx"
};


constexpr const std::uint8_t SM_Corridor032_08_04_Bottom_D[] = {
#embed "../../secret-assets/models/SM_Corridor032_08_04_Bottom_D.fbx"
};


constexpr const std::uint8_t SM_Corridor026_08_02_Top_D_03[] = {
#embed "../../secret-assets/models/SM_Corridor026_08_02_Top_D_03.fbx"
};


constexpr const std::uint8_t SM_Sign_DualArrow[] = {
#embed "../../secret-assets/models/SM_Sign_DualArrow.fbx"
};


constexpr const std::uint8_t SM_Detail14[] = {
#embed "../../secret-assets/models/SM_Detail14.fbx"
};


constexpr const std::uint8_t Display[] = {
#embed "../../secret-assets/models/Display.fbx"
};


constexpr const std::uint8_t SM_Sign_B[] = {
#embed "../../secret-assets/models/SM_Sign_B.fbx"
};


constexpr const std::uint8_t SM_Sign_4[] = {
#embed "../../secret-assets/models/SM_Sign_4.fbx"
};


constexpr const std::uint8_t SM_ServiceTunnel02_04_H[] = {
#embed "../../secret-assets/models/SM_ServiceTunnel02_04_H.fbx"
};


constexpr const std::uint8_t SM_Sign_CommunicationRoom[] = {
#embed "../../secret-assets/models/SM_Sign_CommunicationRoom.fbx"
};


constexpr const std::uint8_t SM_FloorPanel09_03_04[] = {
#embed "../../secret-assets/models/SM_FloorPanel09_03_04.fbx"
};


constexpr const std::uint8_t SM_Closer04[] = {
#embed "../../secret-assets/models/SM_Closer04.fbx"
};


constexpr const std::uint8_t SM_Sign_I[] = {
#embed "../../secret-assets/models/SM_Sign_I.fbx"
};


constexpr const std::uint8_t SM_Door01A02[] = {
#embed "../../secret-assets/models/SM_Door01A02.fbx"
};


constexpr const std::uint8_t SM_Door03_Frame[] = {
#embed "../../secret-assets/models/SM_Door03_Frame.fbx"
};


constexpr const std::uint8_t SM_Corner04_12_12_B_T[] = {
#embed "../../secret-assets/models/SM_Corner04_12_12_B_T.fbx"
};


constexpr const std::uint8_t SM_Corridor013_04_04[] = {
#embed "../../secret-assets/models/SM_Corridor013_04_04.fbx"
};


constexpr const std::uint8_t SM_Details20[] = {
#embed "../../secret-assets/models/SM_Details20.fbx"
};


constexpr const std::uint8_t SM_Corridor047_04_04_ServiceTunnelDoor[] = {
#embed "../../secret-assets/models/SM_Corridor047_04_04_ServiceTunnelDoor.fbx"
};


constexpr const std::uint8_t SM_Closer01[] = {
#embed "../../secret-assets/models/SM_Closer01.fbx"
};


constexpr const std::uint8_t SM_SignBase001[] = {
#embed "../../secret-assets/models/SM_SignBase001.fbx"
};


constexpr const std::uint8_t SM_Door02[] = {
#embed "../../secret-assets/models/SM_Door02.fbx"
};


constexpr const std::uint8_t SM_FloorPanel07_03_04[] = {
#embed "../../secret-assets/models/SM_FloorPanel07_03_04.fbx"
};


constexpr const std::uint8_t SM_Detail18[] = {
#embed "../../secret-assets/models/SM_Detail18.fbx"
};


constexpr const std::uint8_t SM_Door02FrameLight01[] = {
#embed "../../secret-assets/models/SM_Door02FrameLight01.fbx"
};


constexpr const std::uint8_t SM_Door02_LOCK_C[] = {
#embed "../../secret-assets/models/SM_Door02_LOCK_C.fbx"
};


constexpr const std::uint8_t SM_Sign_Zone[] = {
#embed "../../secret-assets/models/SM_Sign_Zone.fbx"
};


constexpr const std::uint8_t SM_Sign_L[] = {
#embed "../../secret-assets/models/SM_Sign_L.fbx"
};


constexpr const std::uint8_t SM_Detail02[] = {
#embed "../../secret-assets/models/SM_Detail02.fbx"
};


constexpr const std::uint8_t SM_Sign_Hangar[] = {
#embed "../../secret-assets/models/SM_Sign_Hangar.fbx"
};


template <class T>
auto to_container(std::span<const std::uint8_t> data) -> T
{
    static_assert(sizeof(typename T::value_type) == 1);

    const auto *ptr = reinterpret_cast<const T::value_type *>(data.data());
    return T{ptr, ptr + data.size()};
}
}

namespace ufps
{

EmbeddedResourceLoader::EmbeddedResourceLoader()
{
    lookup_ = {
                {"textures\\copper_albedo.png", copper_albedo},
        {"textures\\diamond_floor_specular.png", diamond_floor_specular},
        {"textures\\copper_normal.png", copper_normal},
        {"textures\\diamond_floor_normal.png", diamond_floor_normal},
        {"textures\\rusty_metal_normal.png", rusty_metal_normal},
        {"textures\\diamond_floor_albedo.png", diamond_floor_albedo},
        {"textures\\rusty_metal_albedo.png", rusty_metal_albedo},
        {"textures\\rusty_metal_specular.png", rusty_metal_specular},
        {"textures\\copper_specular.png", copper_specular},
        {"shaders\\simple.vert", simple_vert},
        {"shaders\\light_pass.frag", light_pass_frag},
        {"shaders\\gbuffer.vert", gbuffer_vert},
        {"shaders\\gbuffer.frag", gbuffer_frag},
        {"shaders\\simple.frag", simple_frag},
        {"shaders\\light_pass.vert", light_pass_vert},
        {"textures\\MetalPlate03_Height.png", MetalPlate03_Height},
        {"textures\\DeatilsBG_Normal.png", DeatilsBG_Normal},
        {"textures\\Display_BaseColor.png", Display_BaseColor},
        {"textures\\Light02Red_Roughness.png", Light02Red_Roughness},
        {"textures\\Door01_BaseColor.png", Door01_BaseColor},
        {"textures\\Door02_BaseColor.png", Door02_BaseColor},
        {"textures\\Display_Height.png", Display_Height},
        {"textures\\Generator_Metallic.png", Generator_Metallic},
        {"textures\\Display_Normal.png", Display_Normal},
        {"textures\\Glass01_Metallic.png", Glass01_Metallic},
        {"textures\\T_Station01_Metalic.png", T_Station01_Metalic},
        {"textures\\Planet_BC.png", Planet_BC},
        {"textures\\Door03_Roughness.png", Door03_Roughness},
        {"textures\\Door02_Height.png", Door02_Height},
        {"textures\\Door02_Normal.png", Door02_Normal},
        {"textures\\MetalPlate01_Roughness.png", MetalPlate01_Roughness},
        {"textures\\MetalPlate02_BaseColor.png", MetalPlate02_BaseColor},
        {"textures\\Detail02_BaseColor.png", Detail02_BaseColor},
        {"textures\\Planet_R.png", Planet_R},
        {"textures\\Door03_BaseColor.png", Door03_BaseColor},
        {"textures\\Door01_AO.png", Door01_AO},
        {"textures\\Panel14Details_BaseColor.png", Panel14Details_BaseColor},
        {"textures\\Planet_N.png", Planet_N},
        {"textures\\Generator_Emissive.png", Generator_Emissive},
        {"textures\\Light02Red_Metallic.png", Light02Red_Metallic},
        {"textures\\Door01_Normal.png", Door01_Normal},
        {"textures\\Panel14Details_Emissive.png", Panel14Details_Emissive},
        {"textures\\DeatilsBG_BaseColor.png", DeatilsBG_BaseColor},
        {"textures\\Details01_BaseColor.png", Details01_BaseColor},
        {"textures\\Elevator_Metallic.png", Elevator_Metallic},
        {"textures\\MetalPlate03_Roughness.png", MetalPlate03_Roughness},
        {"textures\\Door03_Normal.png", Door03_Normal},
        {"textures\\Light01Blue_Metallic.png", Light01Blue_Metallic},
        {"textures\\Planet_AO.png", Planet_AO},
        {"textures\\T_Station01_Normal.png", T_Station01_Normal},
        {"textures\\Generator_Roughness.png", Generator_Roughness},
        {"textures\\Elevator_Normal.png", Elevator_Normal},
        {"textures\\Panel14Details_Height.png", Panel14Details_Height},
        {"textures\\Door01_Emissive.png", Door01_Emissive},
        {"textures\\T_Station01_Emission.png", T_Station01_Emission},
        {"textures\\Generator_AO.png", Generator_AO},
        {"textures\\Details03_Roughness.png", Details03_Roughness},
        {"textures\\Glass01_Roughness.png", Glass01_Roughness},
        {"textures\\MetalPlate02_Normal.png", MetalPlate02_Normal},
        {"textures\\Details01_Metallic.png", Details01_Metallic},
        {"textures\\Elevator_Roughness.png", Elevator_Roughness},
        {"textures\\MetalPlate02_Roughness.png", MetalPlate02_Roughness},
        {"textures\\Door03_Metallic.png", Door03_Metallic},
        {"textures\\Details03_Metallic.png", Details03_Metallic},
        {"textures\\Detail02_Normal.png", Detail02_Normal},
        {"textures\\Details01_Roughness.png", Details01_Roughness},
        {"textures\\MetalPlate02_Metallic.png", MetalPlate02_Metallic},
        {"textures\\Elevator_AO.png", Elevator_AO},
        {"textures\\Glass01_Normal.png", Glass01_Normal},
        {"textures\\Details01_Height.png", Details01_Height},
        {"textures\\Panel14Details_Metallic.png", Panel14Details_Metallic},
        {"textures\\Generator_Height.png", Generator_Height},
        {"textures\\T_Station01_Height.png", T_Station01_Height},
        {"textures\\MetalPlate03_BaseColor.png", MetalPlate03_BaseColor},
        {"textures\\Light02Red_Height.png", Light02Red_Height},
        {"textures\\SolarCell_Height.png", SolarCell_Height},
        {"textures\\Details02_AO.png", Details02_AO},
        {"textures\\Elevator_Height.png", Elevator_Height},
        {"textures\\SolarCell_Normal.png", SolarCell_Normal},
        {"textures\\Door02_AO.png", Door02_AO},
        {"textures\\Light01Blue_Roughness.png", Light01Blue_Roughness},
        {"textures\\Detail02_Roughness.png", Detail02_Roughness},
        {"textures\\MetalPlate01_Normal.png", MetalPlate01_Normal},
        {"textures\\Glass01_BaseColor.png", Glass01_BaseColor},
        {"textures\\T_Door03_AO.png", T_Door03_AO},
        {"textures\\Light02Red_Normal.png", Light02Red_Normal},
        {"textures\\Details03_Normal.png", Details03_Normal},
        {"textures\\Pipes01_Roughness.png", Pipes01_Roughness},
        {"textures\\Door01_Roughness.png", Door01_Roughness},
        {"textures\\MetalPlate01_Height.png", MetalPlate01_Height},
        {"textures\\Panel14Details_AO.png", Panel14Details_AO},
        {"textures\\Display_Emissive.png", Display_Emissive},
        {"textures\\DeatilsBG_Metallic.png", DeatilsBG_Metallic},
        {"textures\\Display_Metallic.png", Display_Metallic},
        {"textures\\MetalPlate02_Height.png", MetalPlate02_Height},
        {"textures\\MetalPlate01_Metallic.png", MetalPlate01_Metallic},
        {"textures\\Pipes01_Height.png", Pipes01_Height},
        {"textures\\MetalPlate03_Metallic.png", MetalPlate03_Metallic},
        {"textures\\Light01Blue_BaseColor.png", Light01Blue_BaseColor},
        {"textures\\Door03_Height.png", Door03_Height},
        {"textures\\DeatilsBG_Roughness.png", DeatilsBG_Roughness},
        {"textures\\Glass01_Height.png", Glass01_Height},
        {"textures\\Generator_BaseColor.png", Generator_BaseColor},
        {"textures\\SolarCell_BaseColor.png", SolarCell_BaseColor},
        {"textures\\Door03_Emissive.png", Door03_Emissive},
        {"textures\\Details01_Normal.png", Details01_Normal},
        {"textures\\Display_Roughness.png", Display_Roughness},
        {"textures\\Pipes01_Metallic.png", Pipes01_Metallic},
        {"textures\\Panel14Details_Roughness.png", Panel14Details_Roughness},
        {"textures\\Light02Red_Emissive.png", Light02Red_Emissive},
        {"textures\\Light01Blue_Height.png", Light01Blue_Height},
        {"textures\\Light02Red_BaseColor.png", Light02Red_BaseColor},
        {"textures\\MetalPlate01_BaseColor.png", MetalPlate01_BaseColor},
        {"textures\\T_Station01_Roughness.png", T_Station01_Roughness},
        {"textures\\Light01Blue_Normal.png", Light01Blue_Normal},
        {"textures\\Details01_AO.png", Details01_AO},
        {"textures\\Door01_Metallic.png", Door01_Metallic},
        {"textures\\MetalPlate03_Normal.png", MetalPlate03_Normal},
        {"textures\\Generator_Normal.png", Generator_Normal},
        {"textures\\Detail02_Height.png", Detail02_Height},
        {"textures\\Door02_Emissive.png", Door02_Emissive},
        {"textures\\Elevator_Emissive.png", Elevator_Emissive},
        {"textures\\SolarCell_Metallic.png", SolarCell_Metallic},
        {"textures\\Door01_Height.png", Door01_Height},
        {"textures\\Elevator_BaseColor.png", Elevator_BaseColor},
        {"textures\\Door02_Roughness.png", Door02_Roughness},
        {"textures\\Detail02_Metallic.png", Detail02_Metallic},
        {"textures\\Door02_Metallic.png", Door02_Metallic},
        {"textures\\Light01Blue_Emissive.png", Light01Blue_Emissive},
        {"textures\\DeatilsBG_Height.png", DeatilsBG_Height},
        {"textures\\T_Light_BC.png", T_Light_BC},
        {"textures\\T_Light_N.png", T_Light_N},
        {"textures\\T_Station01_BC.png", T_Station01_BC},
        {"textures\\Panel14Details_Normal.png", Panel14Details_Normal},
        {"textures\\Pipes01_BaseColor.png", Pipes01_BaseColor},
        {"textures\\Details03_BaseColor.png", Details03_BaseColor},
        {"textures\\Details01_Emissive.png", Details01_Emissive},
        {"textures\\Details03_Height.png", Details03_Height},
        {"textures\\Pipes01_Normal.png", Pipes01_Normal},
        {"textures\\Detail02_Emissive.png", Detail02_Emissive},
        {"textures\\SolarCell_Roughness.png", SolarCell_Roughness},
        {"models\\SM_Sign_A.fbx", SM_Sign_A},
        {"models\\SM_Corridor033_12_06_Bottom_B.fbx", SM_Corridor033_12_06_Bottom_B},
        {"models\\SM_Details31.fbx", SM_Details31},
        {"models\\SM_Station.fbx", SM_Station},
        {"models\\SM_Corridor019_06_08_B.fbx", SM_Corridor019_06_08_B},
        {"models\\SM_Corner05_8_8_A_L.fbx", SM_Corner05_8_8_A_L},
        {"models\\SM_Panel13.fbx", SM_Panel13},
        {"models\\SM_Sign_2.fbx", SM_Sign_2},
        {"models\\SM_Sign_V.fbx", SM_Sign_V},
        {"models\\SM_Corridor043_12_06_Bottom_B.fbx", SM_Corridor043_12_06_Bottom_B},
        {"models\\SM_Corridor050_02_04_A.fbx", SM_Corridor050_02_04_A},
        {"models\\SM_Sign_W.fbx", SM_Sign_W},
        {"models\\SM_Light013.fbx", SM_Light013},
        {"models\\SM_Door03FramLock02.fbx", SM_Door03FramLock02},
        {"models\\SM_Axis01.fbx", SM_Axis01},
        {"models\\SM_ElevatorEnd.fbx", SM_ElevatorEnd},
        {"models\\SM_Sign_N.fbx", SM_Sign_N},
        {"models\\SM_Detail16.fbx", SM_Detail16},
        {"models\\SM_Sign_ElevatorLight.fbx", SM_Sign_ElevatorLight},
        {"models\\SM_Monitor.fbx", SM_Monitor},
        {"models\\SM_Door03FramLock04.fbx", SM_Door03FramLock04},
        {"models\\SM_CorridorEnd11_E.fbx", SM_CorridorEnd11_E},
        {"models\\SM_FloorDoor.fbx", SM_FloorDoor},
        {"models\\SM_Generator_Wheel03.fbx", SM_Generator_Wheel03},
        {"models\\SM_Sign_Cargo.fbx", SM_Sign_Cargo},
        {"models\\SM_Corridor015_06_08_B.fbx", SM_Corridor015_06_08_B},
        {"models\\SM_Panel02.fbx", SM_Panel02},
        {"models\\SM_Door03B_Light.fbx", SM_Door03B_Light},
        {"models\\SM_Sign_DangerPicture1.fbx", SM_Sign_DangerPicture1},
        {"models\\SM_panel03.fbx", SM_panel03},
        {"models\\SM_FloorPanel08_02_02.fbx", SM_FloorPanel08_02_02},
        {"models\\SM_Windows01_12_8.fbx", SM_Windows01_12_8},
        {"models\\SM_CelilingPanel03_03_04.fbx", SM_CelilingPanel03_03_04},
        {"models\\SM_Stairs01_02_04.fbx", SM_Stairs01_02_04},
        {"models\\SM_Light005.fbx", SM_Light005},
        {"models\\SM_ElevatorTunnelNoLevel.fbx", SM_ElevatorTunnelNoLevel},
        {"models\\SM_ServiceTuinnelDoor01.fbx", SM_ServiceTuinnelDoor01},
        {"models\\SM_Sign_C.fbx", SM_Sign_C},
        {"models\\SM_FloorPanel03_02_02.fbx", SM_FloorPanel03_02_02},
        {"models\\SM_Sign_DangerPicture2.fbx", SM_Sign_DangerPicture2},
        {"models\\SM_Door01Frame.fbx", SM_Door01Frame},
        {"models\\SM_Detail13.fbx", SM_Detail13},
        {"models\\SM_Sign_Danger.fbx", SM_Sign_Danger},
        {"models\\SM_Panel07.fbx", SM_Panel07},
        {"models\\SM_Windows02_12_6.fbx", SM_Windows02_12_6},
        {"models\\SM_CeilingDoor.fbx", SM_CeilingDoor},
        {"models\\SM_Sign_Board.fbx", SM_Sign_Board},
        {"models\\SM_Corridor040_12_06_Top_B_00.fbx", SM_Corridor040_12_06_Top_B_00},
        {"models\\SM_Fan02.fbx", SM_Fan02},
        {"models\\SM_Fan03.fbx", SM_Fan03},
        {"models\\SM_SignRoom.fbx", SM_SignRoom},
        {"models\\SM_Sign_Level.fbx", SM_Sign_Level},
        {"models\\SM_CorridorEnd02_A.fbx", SM_CorridorEnd02_A},
        {"models\\SM_Sign_D.fbx", SM_Sign_D},
        {"models\\SM_SignDanger1.fbx", SM_SignDanger1},
        {"models\\SM_Sign_Elevator.fbx", SM_Sign_Elevator},
        {"models\\SM_FloorPanel16_04_02.fbx", SM_FloorPanel16_04_02},
        {"models\\SM_Sign_Deck.fbx", SM_Sign_Deck},
        {"models\\SM_Detail07.fbx", SM_Detail07},
        {"models\\SM_Sign_H.fbx", SM_Sign_H},
        {"models\\SM_CorridorEnd06_D.fbx", SM_CorridorEnd06_D},
        {"models\\SM_Detail10.fbx", SM_Detail10},
        {"models\\SM_Gate01_A_B.fbx", SM_Gate01_A_B},
        {"models\\SM_Door03B.fbx", SM_Door03B},
        {"models\\SM_Light007.fbx", SM_Light007},
        {"models\\SM_Details35.fbx", SM_Details35},
        {"models\\SM_ServiceTunnel05_04_HV.fbx", SM_ServiceTunnel05_04_HV},
        {"models\\SM_Corridor037_12_06_Top_B_04.fbx", SM_Corridor037_12_06_Top_B_04},
        {"models\\SM_Corridor008_06_08_B.fbx", SM_Corridor008_06_08_B},
        {"models\\SM_Light011.fbx", SM_Light011},
        {"models\\SM_Details33.fbx", SM_Details33},
        {"models\\SM_Sign_K.fbx", SM_Sign_K},
        {"models\\SM_Corridor021_08_02_Top_D_03.fbx", SM_Corridor021_08_02_Top_D_03},
        {"models\\SM_Panel10.fbx", SM_Panel10},
        {"models\\SM_Sign_E.fbx", SM_Sign_E},
        {"models\\SM_Corridor041_12_06_Top_B_04.fbx", SM_Corridor041_12_06_Top_B_04},
        {"models\\SM_Corridor017_04_04_A.fbx", SM_Corridor017_04_04_A},
        {"models\\SM_Sign_Q.fbx", SM_Sign_Q},
        {"models\\SM_FloorPanel05_02_04.fbx", SM_FloorPanel05_02_04},
        {"models\\SM_CelilingPanel04_03_04.fbx", SM_CelilingPanel04_03_04},
        {"models\\SM_Sign_6.fbx", SM_Sign_6},
        {"models\\SM_SignCaution1.fbx", SM_SignCaution1},
        {"models\\SM_Sign_3.fbx", SM_Sign_3},
        {"models\\SM_Generator_Wheel01.fbx", SM_Generator_Wheel01},
        {"models\\SM_Corridor046_06_08_ServiceTunnelDoor.fbx", SM_Corridor046_06_08_ServiceTunnelDoor},
        {"models\\SM_Corridor046_04_04.fbx", SM_Corridor046_04_04},
        {"models\\SM_Corridor014_04_04.fbx", SM_Corridor014_04_04},
        {"models\\SM_Corner01_8_8_X.fbx", SM_Corner01_8_8_X},
        {"models\\SM_Sign_O.fbx", SM_Sign_O},
        {"models\\SM_ElevatorDoor04.fbx", SM_ElevatorDoor04},
        {"models\\SM_Windows04_8_6.fbx", SM_Windows04_8_6},
        {"models\\SM_CorridorEnd04_B.fbx", SM_CorridorEnd04_B},
        {"models\\SM_Closer03.fbx", SM_Closer03},
        {"models\\SM_Detail03.fbx", SM_Detail03},
        {"models\\SM_Panel14UE.fbx", SM_Panel14UE},
        {"models\\SM_Sign_Port.fbx", SM_Sign_Port},
        {"models\\SM_Details26.fbx", SM_Details26},
        {"models\\SM_Elevator.fbx", SM_Elevator},
        {"models\\SM_FloorPanel14_02_01.fbx", SM_FloorPanel14_02_01},
        {"models\\SM_Windows03_12_4.fbx", SM_Windows03_12_4},
        {"models\\SM_SignBase002.fbx", SM_SignBase002},
        {"models\\SM_Gear02.fbx", SM_Gear02},
        {"models\\SM_FloorPanel12_01_01.fbx", SM_FloorPanel12_01_01},
        {"models\\SM_Generator.fbx", SM_Generator},
        {"models\\SM_Light001.fbx", SM_Light001},
        {"models\\SM_Sign_S.fbx", SM_Sign_S},
        {"models\\SM_Panel08.fbx", SM_Panel08},
        {"models\\SM_Details30.fbx", SM_Details30},
        {"models\\SM_Corner02_8_8_A_T.fbx", SM_Corner02_8_8_A_T},
        {"models\\SM_Door03A_Lock.fbx", SM_Door03A_Lock},
        {"models\\SM_Corridor009_06_08_B.fbx", SM_Corridor009_06_08_B},
        {"models\\SM_Corridor053_08_08_A.fbx", SM_Corridor053_08_08_A},
        {"models\\SM_Details27.fbx", SM_Details27},
        {"models\\SM_Sign_G.fbx", SM_Sign_G},
        {"models\\SM_Corridor029_08_04_Top_D_01.fbx", SM_Corridor029_08_04_Top_D_01},
        {"models\\SM_FloorPanel10_07_04.fbx", SM_FloorPanel10_07_04},
        {"models\\SM_Light002.fbx", SM_Light002},
        {"models\\SM_Sign_EngineRoom.fbx", SM_Sign_EngineRoom},
        {"models\\SM_Door03A_Light.fbx", SM_Door03A_Light},
        {"models\\SM_Sign_Caution.fbx", SM_Sign_Caution},
        {"models\\SM_Corridor031_08_04_Bottom_D.fbx", SM_Corridor031_08_04_Bottom_D},
        {"models\\SM_Sign_F.fbx", SM_Sign_F},
        {"models\\SM_Corridor023_08_04_Bottom_D.fbx", SM_Corridor023_08_04_Bottom_D},
        {"models\\SM_Corridor039_12_06_Top_B_02.fbx", SM_Corridor039_12_06_Top_B_02},
        {"models\\SM_Details21.fbx", SM_Details21},
        {"models\\SM_Corridor051_02_04_A.fbx", SM_Corridor051_02_04_A},
        {"models\\SM_Light009Light.fbx", SM_Light009Light},
        {"models\\SM_Corridor028_08_04_Bottom_D.fbx", SM_Corridor028_08_04_Bottom_D},
        {"models\\SM_CorridorEnd05_D.fbx", SM_CorridorEnd05_D},
        {"models\\SM_CorridorEnd07_D.fbx", SM_CorridorEnd07_D},
        {"models\\SM_Sign_9.fbx", SM_Sign_9},
        {"models\\SM_Detail05.fbx", SM_Detail05},
        {"models\\SM_CelilingPanel01_04_02.fbx", SM_CelilingPanel01_04_02},
        {"models\\SM_Sign_M.fbx", SM_Sign_M},
        {"models\\SM_Sign_P.fbx", SM_Sign_P},
        {"models\\SM_ServiceTunnel01_04_H.fbx", SM_ServiceTunnel01_04_H},
        {"models\\SM_FloorPanel11_03_03.fbx", SM_FloorPanel11_03_03},
        {"models\\SM_Corridor018_06_08_B.fbx", SM_Corridor018_06_08_B},
        {"models\\SM_Light006.fbx", SM_Light006},
        {"models\\SM_Axis02.fbx", SM_Axis02},
        {"models\\SM_Sign_Gate.fbx", SM_Sign_Gate},
        {"models\\SM_Details24.fbx", SM_Details24},
        {"models\\SM_Corridor020_08_02_Bottom_D.fbx", SM_Corridor020_08_02_Bottom_D},
        {"models\\SM_Sign_RightArrow.fbx", SM_Sign_RightArrow},
        {"models\\SM_Windows05_6_4.fbx", SM_Windows05_6_4},
        {"models\\SM_Corner03_12_12_B_X.fbx", SM_Corner03_12_12_B_X},
        {"models\\SM_Corridor024_08_04_Top_D_01.fbx", SM_Corridor024_08_04_Top_D_01},
        {"models\\SM_Corridor010_02_02.fbx", SM_Corridor010_02_02},
        {"models\\SM_Stairs03_02_04.fbx", SM_Stairs03_02_04},
        {"models\\SM_Details32.fbx", SM_Details32},
        {"models\\SM_Sign_TechnicalRoom.fbx", SM_Sign_TechnicalRoom},
        {"models\\SM_ElevatorDoor01.fbx", SM_ElevatorDoor01},
        {"models\\SM_Corridor_Stairs01_04_02.fbx", SM_Corridor_Stairs01_04_02},
        {"models\\SM_Corridor052_02_04_A.fbx", SM_Corridor052_02_04_A},
        {"models\\SM_Door03FramLock01.fbx", SM_Door03FramLock01},
        {"models\\SM_ElevatorCabin.fbx", SM_ElevatorCabin},
        {"models\\SM_Details38.fbx", SM_Details38},
        {"models\\SM_Corridor003_04_04_A.fbx", SM_Corridor003_04_04_A},
        {"models\\SM_Door03B_Lock.fbx", SM_Door03B_Lock},
        {"models\\SM_ElevatorDoor02.fbx", SM_ElevatorDoor02},
        {"models\\SM_Detail11.fbx", SM_Detail11},
        {"models\\SM_Sign_J.fbx", SM_Sign_J},
        {"models\\SM_Details22.fbx", SM_Details22},
        {"models\\SM_CorridorEnd09_E.fbx", SM_CorridorEnd09_E},
        {"models\\SM_Panel09.fbx", SM_Panel09},
        {"models\\SM_Sign_Prior.fbx", SM_Sign_Prior},
        {"models\\SM_Sign_7.fbx", SM_Sign_7},
        {"models\\SM_Corridor042_12_06_Bottom_B.fbx", SM_Corridor042_12_06_Bottom_B},
        {"models\\SM_Door02_LOCK_B.fbx", SM_Door02_LOCK_B},
        {"models\\SM_Details34.fbx", SM_Details34},
        {"models\\SM_Details19.fbx", SM_Details19},
        {"models\\SM_Corridor049_08_04_ServiceTunnelDoor.fbx", SM_Corridor049_08_04_ServiceTunnelDoor},
        {"models\\SM_Detail06.fbx", SM_Detail06},
        {"models\\SM_Corridor012_02_02.fbx", SM_Corridor012_02_02},
        {"models\\SM_Corridor025_08_04_Top_D_03.fbx", SM_Corridor025_08_04_Top_D_03},
        {"models\\SM_Detail12.fbx", SM_Detail12},
        {"models\\SM_ServiceTuinnelDoor02.fbx", SM_ServiceTuinnelDoor02},
        {"models\\SM_Light010.fbx", SM_Light010},
        {"models\\SM_Sign_MedicalRoom.fbx", SM_Sign_MedicalRoom},
        {"models\\SM_Detail15.fbx", SM_Detail15},
        {"models\\SM_Gear01.fbx", SM_Gear01},
        {"models\\SM_Door01B01.fbx", SM_Door01B01},
        {"models\\SM_FloorPanel01_01_01.fbx", SM_FloorPanel01_01_01},
        {"models\\SM_Details25.fbx", SM_Details25},
        {"models\\SM_Light003.fbx", SM_Light003},
        {"models\\SM_Light008.fbx", SM_Light008},
        {"models\\SM_CorridorEnd08_E.fbx", SM_CorridorEnd08_E},
        {"models\\SM_FloorPanel15_03_03.fbx", SM_FloorPanel15_03_03},
        {"models\\SM_ServiceTunnel08_04_VH.fbx", SM_ServiceTunnel08_04_VH},
        {"models\\SM_Detail17.fbx", SM_Detail17},
        {"models\\SM_ServiceTunnel06_04_V.fbx", SM_ServiceTunnel06_04_V},
        {"models\\SM_Details28.fbx", SM_Details28},
        {"models\\SM_ServiceTunnel04_04_HC.fbx", SM_ServiceTunnel04_04_HC},
        {"models\\SM_Corridor011_02_03.fbx", SM_Corridor011_02_03},
        {"models\\SM_Corridor005_04_04_A.fbx", SM_Corridor005_04_04_A},
        {"models\\SM_Corridor007_06_08_B.fbx", SM_Corridor007_06_08_B},
        {"models\\SM_Door01A01.fbx", SM_Door01A01},
        {"models\\SM_Corridor044_12_Closer_B.fbx", SM_Corridor044_12_Closer_B},
        {"models\\SM_Closer02.fbx", SM_Closer02},
        {"models\\SM_Light009.fbx", SM_Light009},
        {"models\\SM_SignHighVoltage1.fbx", SM_SignHighVoltage1},
        {"models\\SM_Sign__.fbx", SM_Sign__},
        {"models\\SM_CorridorEnd01_A.fbx", SM_CorridorEnd01_A},
        {"models\\SM_Panel05.fbx", SM_Panel05},
        {"models\\SM_Corridor035_12_06_Top_B_00.fbx", SM_Corridor035_12_06_Top_B_00},
        {"models\\SM_Door01B02.fbx", SM_Door01B02},
        {"models\\SM_Door02_Light01.fbx", SM_Door02_Light01},
        {"models\\SM_CelilingPanel02_04_04.fbx", SM_CelilingPanel02_04_04},
        {"models\\SM_Sign_8.fbx", SM_Sign_8},
        {"models\\SM_Corridor030_08_04_Top_D_03.fbx", SM_Corridor030_08_04_Top_D_03},
        {"models\\SM_Details37.fbx", SM_Details37},
        {"models\\SM_Sign_Sector.fbx", SM_Sign_Sector},
        {"models\\SM_Corridor036_12_06_Top_B_02.fbx", SM_Corridor036_12_06_Top_B_02},
        {"models\\SM_Sign_5.fbx", SM_Sign_5},
        {"models\\SM_Corridor004_04_04_A.fbx", SM_Corridor004_04_04_A},
        {"models\\SM_Details36.fbx", SM_Details36},
        {"models\\SM_Stairs02_02_02.fbx", SM_Stairs02_02_02},
        {"models\\SM_Details29.fbx", SM_Details29},
        {"models\\SM_Door03A.fbx", SM_Door03A},
        {"models\\SM_FloorPanel06_005_02.fbx", SM_FloorPanel06_005_02},
        {"models\\SM_CorridorEnd03_B.fbx", SM_CorridorEnd03_B},
        {"models\\SM_Panel12.fbx", SM_Panel12},
        {"models\\SM_Corridor022_08_02_Top_D_01.fbx", SM_Corridor022_08_02_Top_D_01},
        {"models\\SM_ElevatorDoor03.fbx", SM_ElevatorDoor03},
        {"models\\SM_Corridor046_06_08_Light.fbx", SM_Corridor046_06_08_Light},
        {"models\\SM_CorridorEnd10_E.fbx", SM_CorridorEnd10_E},
        {"models\\SM_ServiceTunnel03_04_H.fbx", SM_ServiceTunnel03_04_H},
        {"models\\SM_Corridor006_04_08_A.fbx", SM_Corridor006_04_08_A},
        {"models\\SM_Light012.fbx", SM_Light012},
        {"models\\SM_ServiceTunnel07_04_V.fbx", SM_ServiceTunnel07_04_V},
        {"models\\SM_Door02_LOCK_A.fbx", SM_Door02_LOCK_A},
        {"models\\SM_Panel11.fbx", SM_Panel11},
        {"models\\SM_Detail08.fbx", SM_Detail08},
        {"models\\SM_Corridor013_02_02.fbx", SM_Corridor013_02_02},
        {"models\\Panel14Light.fbx", Panel14Light},
        {"models\\SM_Detail04.fbx", SM_Detail04},
        {"models\\SM_FloorPanel13_04_01.fbx", SM_FloorPanel13_04_01},
        {"models\\SM_Gear03.fbx", SM_Gear03},
        {"models\\SM_SolarCell.fbx", SM_SolarCell},
        {"models\\SM_Planet.fbx", SM_Planet},
        {"models\\SM_Panel04.fbx", SM_Panel04},
        {"models\\SM_Gear04.fbx", SM_Gear04},
        {"models\\SM_Corridor002_04_02_A.fbx", SM_Corridor002_04_02_A},
        {"models\\SM_Corner06_12_12_B_L.fbx", SM_Corner06_12_12_B_L},
        {"models\\SM_Detail09.fbx", SM_Detail09},
        {"models\\SM_Sign_T.fbx", SM_Sign_T},
        {"models\\SM_Details23.fbx", SM_Details23},
        {"models\\SM_Door03FramLock03.fbx", SM_Door03FramLock03},
        {"models\\SM_Gate01_A_B_D.fbx", SM_Gate01_A_B_D},
        {"models\\SM_Sign_U.fbx", SM_Sign_U},
        {"models\\SM_Sign_Door.fbx", SM_Sign_Door},
        {"models\\SM_Details23Animated.fbx", SM_Details23Animated},
        {"models\\SM_Sign_Lock.fbx", SM_Sign_Lock},
        {"models\\SM_Light004.fbx", SM_Light004},
        {"models\\SM_Sign_MaintenenceRoom.fbx", SM_Sign_MaintenenceRoom},
        {"models\\SM_Sign_1.fbx", SM_Sign_1},
        {"models\\SM_Generator_Wheel02.fbx", SM_Generator_Wheel02},
        {"models\\SM_Sign_R.fbx", SM_Sign_R},
        {"models\\SM_Fan01.fbx", SM_Fan01},
        {"models\\SM_Corridor038_12_06_Bottom_B.fbx", SM_Corridor038_12_06_Bottom_B},
        {"models\\SM_Door01Frame01_A_B.fbx", SM_Door01Frame01_A_B},
        {"models\\SM_Corridor027_08_02_Top_D_01.fbx", SM_Corridor027_08_02_Top_D_01},
        {"models\\SM_Corridor045_04_04.fbx", SM_Corridor045_04_04},
        {"models\\SM_Panel06.fbx", SM_Panel06},
        {"models\\SM_Corridor016_04_04_A.fbx", SM_Corridor016_04_04_A},
        {"models\\SM_Sign_0.fbx", SM_Sign_0},
        {"models\\SM_Sign__LeftArrow.fbx", SM_Sign__LeftArrow},
        {"models\\SM_Corridor001_04_02_A.fbx", SM_Corridor001_04_02_A},
        {"models\\SM_Panel14.fbx", SM_Panel14},
        {"models\\SM_Panel01.fbx", SM_Panel01},
        {"models\\SM_FloorPanel04_005_02.fbx", SM_FloorPanel04_005_02},
        {"models\\SM_Detail01.fbx", SM_Detail01},
        {"models\\SM_Corridor032_08_04_Bottom_D.fbx", SM_Corridor032_08_04_Bottom_D},
        {"models\\SM_Corridor026_08_02_Top_D_03.fbx", SM_Corridor026_08_02_Top_D_03},
        {"models\\SM_Sign_DualArrow.fbx", SM_Sign_DualArrow},
        {"models\\SM_Detail14.fbx", SM_Detail14},
        {"models\\Display.fbx", Display},
        {"models\\SM_Sign_B.fbx", SM_Sign_B},
        {"models\\SM_Sign_4.fbx", SM_Sign_4},
        {"models\\SM_ServiceTunnel02_04_H.fbx", SM_ServiceTunnel02_04_H},
        {"models\\SM_Sign_CommunicationRoom.fbx", SM_Sign_CommunicationRoom},
        {"models\\SM_FloorPanel09_03_04.fbx", SM_FloorPanel09_03_04},
        {"models\\SM_Closer04.fbx", SM_Closer04},
        {"models\\SM_Sign_I.fbx", SM_Sign_I},
        {"models\\SM_Door01A02.fbx", SM_Door01A02},
        {"models\\SM_Door03_Frame.fbx", SM_Door03_Frame},
        {"models\\SM_Corner04_12_12_B_T.fbx", SM_Corner04_12_12_B_T},
        {"models\\SM_Corridor013_04_04.fbx", SM_Corridor013_04_04},
        {"models\\SM_Details20.fbx", SM_Details20},
        {"models\\SM_Corridor047_04_04_ServiceTunnelDoor.fbx", SM_Corridor047_04_04_ServiceTunnelDoor},
        {"models\\SM_Closer01.fbx", SM_Closer01},
        {"models\\SM_SignBase001.fbx", SM_SignBase001},
        {"models\\SM_Door02.fbx", SM_Door02},
        {"models\\SM_FloorPanel07_03_04.fbx", SM_FloorPanel07_03_04},
        {"models\\SM_Detail18.fbx", SM_Detail18},
        {"models\\SM_Door02FrameLight01.fbx", SM_Door02FrameLight01},
        {"models\\SM_Door02_LOCK_C.fbx", SM_Door02_LOCK_C},
        {"models\\SM_Sign_Zone.fbx", SM_Sign_Zone},
        {"models\\SM_Sign_L.fbx", SM_Sign_L},
        {"models\\SM_Detail02.fbx", SM_Detail02},
        {"models\\SM_Sign_Hangar.fbx", SM_Sign_Hangar},
    };
}

auto EmbeddedResourceLoader::load_string(std::string_view name) -> std::string
{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {} does not exist", name);

    return to_container<std::string>(resource->second);
}

auto EmbeddedResourceLoader::load_data_buffer(std::string_view name) -> DataBuffer
{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {} does not exist", name);

    return to_container<DataBuffer>(resource->second);
}
}

