#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("compute", "[Shader]")
{
	SECTION("Simple texture copy")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Data
{
	tex_size: vec2[u32]
}

[auto_binding]
external
{
	input_tex: texture2D[f32, readonly, rgba8],
	output_tex: texture2D[f32, writeonly, rgba8],
	data: uniform[Data]
}

struct Input
{
	[builtin(global_invocation_indices)] indices: vec3[u32]
}

[entry(compute)]
[workgroup(32, 16*2, 1)]
fn main(input: Input)
{
	if (input.indices.x >= data.tex_size.x || input.indices.y >= data.tex_size.y)
		return;

	let value = input_tex.Read(vec2[i32](input.indices.xy));
	output_tex.Write(vec2[i32](input.indices.xy), value);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glES = true;
		glslEnv.glMajorVersion = 3;
		glslEnv.glMinorVersion = 1;

		ExpectGLSL(*shaderModule, R"(
#version 310 es

// compute shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp image2D;
#else
precision mediump float;
precision mediump image2D;
#endif

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// header end

// struct Data omitted (used as UBO/SSBO)

layout(rgba8) uniform readonly image2D input_tex;
layout(rgba8) uniform writeonly image2D output_tex;
uniform _nzslBinding_data
{
	uvec2 tex_size;
} data;

struct Input
{
	uvec3 indices;
};

void main()
{
	Input input_;
	input_.indices = gl_GlobalInvocationID;

	if ((input_.indices.x >= data.tex_size.x) || (input_.indices.y >= data.tex_size.y))
	{
		return;
	}

	vec4 value = imageLoad(input_tex, ivec2(input_.indices.xy));
	imageStore(output_tex, ivec2(input_.indices.xy), value);
})", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

struct Data
{
	tex_size: vec2[u32]
}

[auto_binding(true)]
external
{
	[set(0), binding(0)] input_tex: texture2D[f32, readonly, rgba8],
	[set(0), binding(1)] output_tex: texture2D[f32, writeonly, rgba8],
	[set(0), binding(2)] data: uniform[Data]
}

struct Input
{
	[builtin(global_invocation_indices)] indices: vec3[u32]
}

[entry(comp), workgroup(32, 32, 1)]
fn main(input: Input)
{
	if ((input.indices.x >= data.tex_size.x) || (input.indices.y >= data.tex_size.y))
	{
		return;
	}

	let value: vec4[f32] = input_tex.Read(vec2[i32](input.indices.xy));
	output_tex.Write(vec2[i32](input.indices.xy), value);
})");

		ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(GLCompute) %27 "main" %17
      OpExecutionMode %27 ExecutionMode(LocalSize) 32 32 1
      OpSource SourceLanguage(Unknown) 100
      OpName %3 "Data"
      OpMemberName %3 0 "tex_size"
      OpName %9 "Data"
      OpMemberName %9 0 "tex_size"
      OpName %13 "Input"
      OpMemberName %13 0 "indices"
      OpName %7 "input_tex"
      OpName %8 "output_tex"
      OpName %11 "data"
      OpName %17 "global_invocation_indices"
      OpName %27 "main"
      OpDecorate %7 Decoration(Binding) 0
      OpDecorate %7 Decoration(DescriptorSet) 0
      OpDecorate %8 Decoration(Binding) 1
      OpDecorate %8 Decoration(DescriptorSet) 0
      OpDecorate %11 Decoration(Binding) 2
      OpDecorate %11 Decoration(DescriptorSet) 0
      OpDecorate %17 Decoration(BuiltIn) BuiltIn(GlobalInvocationId)
      OpMemberDecorate %3 0 Decoration(Offset) 0
      OpDecorate %9 Decoration(Block)
      OpMemberDecorate %9 0 Decoration(Offset) 0
      OpMemberDecorate %13 0 Decoration(Offset) 0
 %1 = OpTypeInt 32 0
 %2 = OpTypeVector %1 2
 %3 = OpTypeStruct %2
 %4 = OpTypeFloat 32
 %5 = OpTypeImage %4 Dim(Dim2D) 2 0 0 2 ImageFormat(Rgba8)
 %6 = OpTypePointer StorageClass(UniformConstant) %5
 %9 = OpTypeStruct %2
%10 = OpTypePointer StorageClass(Uniform) %9
%12 = OpTypeVector %1 3
%13 = OpTypeStruct %12
%14 = OpTypeVoid
%15 = OpTypeFunction %14
%16 = OpTypePointer StorageClass(Input) %12
%18 = OpTypeInt 32 1
%19 = OpConstant %18 i32(0)
%20 = OpTypePointer StorageClass(Function) %12
%21 = OpTypePointer StorageClass(Function) %13
%22 = OpTypeBool
%23 = OpConstant %18 i32(1)
%24 = OpTypeVector %18 2
%25 = OpTypeVector %4 4
%26 = OpTypePointer StorageClass(Function) %25
%38 = OpTypePointer StorageClass(Uniform) %2
 %7 = OpVariable %6 StorageClass(UniformConstant)
 %8 = OpVariable %6 StorageClass(UniformConstant)
%11 = OpVariable %10 StorageClass(Uniform)
%17 = OpVariable %16 StorageClass(Input)
%27 = OpFunction %14 FunctionControl(0) %15
%28 = OpLabel
%29 = OpVariable %26 StorageClass(Function)
%30 = OpVariable %21 StorageClass(Function)
%31 = OpAccessChain %20 %30 %19
      OpCopyMemory %31 %17
%35 = OpAccessChain %20 %30 %19
%36 = OpLoad %12 %35
%37 = OpCompositeExtract %1 %36 0
%39 = OpAccessChain %38 %11 %19
%40 = OpLoad %2 %39
%41 = OpCompositeExtract %1 %40 0
%42 = OpUGreaterThanEqual %22 %37 %41
%43 = OpAccessChain %20 %30 %19
%44 = OpLoad %12 %43
%45 = OpCompositeExtract %1 %44 1
%46 = OpAccessChain %38 %11 %19
%47 = OpLoad %2 %46
%48 = OpCompositeExtract %1 %47 1
%49 = OpUGreaterThanEqual %22 %45 %48
%50 = OpLogicalOr %22 %42 %49
      OpSelectionMerge %32 SelectionControl(0)
      OpBranchConditional %50 %33 %34
%33 = OpLabel
      OpReturn
%34 = OpLabel
      OpBranch %32
%32 = OpLabel
%51 = OpLoad %5 %7
%52 = OpAccessChain %20 %30 %19
%53 = OpLoad %12 %52
%54 = OpVectorShuffle %2 %53 %53 0 1
%55 = OpBitcast %24 %54
%56 = OpImageRead %25 %51 %55
      OpStore %29 %56
%57 = OpLoad %5 %8
%58 = OpAccessChain %20 %30 %19
%59 = OpLoad %12 %58
%60 = OpVectorShuffle %2 %59 %59 0 1
%61 = OpBitcast %24 %60
%62 = OpLoad %25 %29
      OpImageWrite %57 %61 %62
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
