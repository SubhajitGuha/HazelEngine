#include "Sandbox2dApp.h"

SandBox2dApp::SandBox2dApp()
	:m_camera(1920.0/1080.0)
{
}

void SandBox2dApp::OnAttach()
{
	float pos[] =
	{ 0.5,0.5,0.0,   1.0,1.0,
	0.5,-0.5,0.0,   1.0,0.0,
	-0.5,-0.5,0.0,  0.0,0.0,
	-0.5,0.5,0.0,   0.0,1.0 };

	unsigned int index[] =
	{ 0,1,2,
	0,2,3 };


	vao .reset(VertexArray::Create());//vertex array
	ref<VertexBuffer> vb(VertexBuffer::Create(pos, sizeof(pos)));//vertex buffer
	ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout
	bl->push("position", DataType::Float3);
	bl->push("color", DataType::Float2);
	ref<IndexBuffer> ib(IndexBuffer::Create(index, sizeof(index)));
	vao->AddBuffer(bl, vb);
	vao->SetIndexBuffer(ib);

	shader.reset(Shader::Create("Assets/Shaders/TextureShader.glsl"));

	tex2 = Texture2D::Create("Assets/Textures/rickshaw.png");
	texture = Texture2D::Create("Assets/Textures/Test.png");

	shader->UploadUniformInt("u_Texture", 0);
}

void SandBox2dApp::OnDetach()
{
}

void SandBox2dApp::OnUpdate(float deltatime)
{
	RenderCommand::ClearColor(glm::vec4(0.4, 0.4, 0.4, 1.0));
	RenderCommand::Clear();

	m_camera.OnUpdate(deltatime);

	if (Input::IsKeyPressed(HZ_KEY_UP))
		position.y += ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_DOWN))
		position.y -= ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_LEFT))
		position.x -= ObjSpeed * deltatime;
	if (Input::IsKeyPressed(HZ_KEY_RIGHT))
		position.x += ObjSpeed * deltatime;
	if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_4))
		scale += 0.3;
	if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_5))
		scale -= 0.3;

	glm::mat4 ModelTransform2 = glm::translate(glm::mat4(1), position) * glm::scale(glm::mat4(1), glm::vec3(scale));
	/*
	Set the model transform for now there is no scale or rotation
	just multiply the scale and rotation matrix with position to get the full transform
	*/
	Renderer::BeginScene(m_camera.GetCamera());	
	texture->Bind(0);
	Renderer::Submit(*shader, *vao, ModelTransform2);
	tex2->Bind(0);
	Renderer::Submit(*shader, *vao);
	Renderer::EndScene();
}

void SandBox2dApp::OnImGuiRender()
{
	ImGui::Begin("Color Picker");
	ImGui::ColorPicker4("Color3", glm::value_ptr(Color1));
	ImGui::End();
}

void SandBox2dApp::OnEvent(Event& e)
{
	m_camera.OnEvent(e);
}
