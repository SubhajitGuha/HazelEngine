#include <Hazel.h>
using namespace Hazel;
class Layerimplement :public Hazel::Layer {
public:
	Layerimplement()
		:Layer("Hazel_Layer"), m_camera(-4,4,4,-4)
	{
		float pos[] =
		{ -0.8,0.8,0.0,  0.8 ,0.0 ,0.6 ,1.0,
		0.8,-0.8,0.0,   0.1 ,0.6 ,0.9 ,1.0,
		-0.8,-0.80,0.0, 0.8 ,0.7 ,0.0 ,1.0,
		0.8,0.8,0.0,    0.2 ,0.9 ,0.8 ,1.0,
		0.0,0.5,0.0,    0.3 ,0.3 ,0.3 ,1.0,
		0.5,-0.5,0.0,   0.3 ,0.3 ,0.3 ,1.0,
		-0.5,-0.5,0.0,  0.3 ,0.3 ,0.3 ,1.0 };

		unsigned int index[] =
		{ 0,1,2,
		0,1,3,
		4,5,6 };

		 vao = VertexArray::Create();//vertex array
		 vb = VertexBuffer::Create(pos, sizeof(pos));//vertex buffer
		BufferLayout bl;
		bl.push("position", DataType::Float3);
		bl.push("color", DataType::Float4);

		 ib = IndexBuffer::Create(index, sizeof(index));

		vao->AddBuffer(bl, *vb);
		vao->SetIndexBuffer((std::shared_ptr<IndexBuffer>)ib);

		std::string vertexshader = R"(
			#version 410 core
			layout (location = 0) in vec3 pos;
			layout (location = 1) in vec4 col;

			out vec4 m_color;
			uniform mat4 mvp;
			void main()
			{
				m_color = col;
				gl_Position = mvp * vec4(pos ,1.0);
				
			}
		)";
		std::string fragmentshader = R"(
			#version 410 core
			layout (location = 0) out vec4 color;

			in vec4 m_color;

			void main()
			{
				color= m_color;
			}
		)";
		shader = new Shader(vertexshader, fragmentshader);

		m_camera.SetPosition({ 0.0, 0, 0 });//set the location of the model
		m_camera.SetRotation(30);// set the rotation of the model (in z axis (hard codded))

	}
	virtual void OnUpdate() {
		
		if (Input::IsKeyPressed(HZ_KEY_W))
			v3.y += m_movespeed;
		if (Input::IsKeyPressed(HZ_KEY_S))
			v3.y -= m_movespeed;
		if (Input::IsKeyPressed(HZ_KEY_A))
			v3.x += m_movespeed;
		if (Input::IsKeyPressed(HZ_KEY_D))
			v3.x -= m_movespeed;
		if (Input::IsKeyPressed(HZ_KEY_E))
			r += 2;
		if (Input::IsKeyPressed(HZ_KEY_Q))
			r -= 2;

		RenderCommand::ClearColor(glm::vec4(0.8, 0.8, 0.8, 0.8));
		RenderCommand::Clear();

		m_camera.SetPosition(v3);
		m_camera.SetRotation(r);

		Renderer::BeginScene(m_camera);
		shader->UploadUniformMat4("mvp", Renderer::m_data->m_ProjectionViewMatrix);
		Renderer::Submit(*vao);
		Renderer::EndScene();
	}

	void OnImGuiRender() {
	}

	virtual void OnEvent(Hazel::Event& e) {
	}

public:
	glm::vec3 v3={0,0,0};
	float m_movespeed = 0.2;
	float r = 0;
private:
	bool m_Running = true;
	Shader* shader;
	OrthographicCamera m_camera;
	VertexArray* vao;
	VertexBuffer* vb;
	IndexBuffer* ib;
};

class Sandbox :public Hazel::Application
{
public:
	Sandbox(){
		PushLayer(new Layerimplement());
		//PushOverlay(new Hazel::ImGuiLayer());
	}
	~Sandbox(){}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
