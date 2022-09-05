#include <Hazel.h>
using namespace Hazel;

//This is our game layer
class GameLayer :public Hazel::Layer {
public:
	GameLayer()
		:Layer("Hazel_Layer"), m_camera(-4,4,-4,4)//left right bottom top
	{
		float pos[] =
		{0.0,0.5,0.0,   0.8 ,0.0 ,0.6 ,1.0,
		0.5,-0.5,0.0,   0.1 ,0.6 ,0.9 ,1.0,
		-0.5,-0.5,0.0,  0.8 ,0.7 ,0.0 ,1.0};
	
		unsigned int index[] =
		{0,1,2};

		
		vao.reset(VertexArray::Create());//vertex array
		ref<VertexBuffer> vb(VertexBuffer::Create(pos, sizeof(pos) ));//vertex buffer
		ref<BufferLayout> bl(new BufferLayout()); //buffer layout
		bl->push("position", DataType::Float3);
		bl->push("color", DataType::Float4);
		ref<IndexBuffer> ib(IndexBuffer::Create(index, sizeof(index)));
		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		std::string vertexshader = R"(
			#version 410 core
			layout (location = 0) in vec3 pos;
			layout (location = 1) in vec4 col;

			out vec4 m_color;
			uniform mat4 m_ProjectionView;
			uniform mat4 m_ModelTransform;
			void main()
			{
				m_color = col;
				gl_Position = m_ProjectionView * m_ModelTransform * vec4(pos ,1.0);
				
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
		shader = Shader::Create(vertexshader, fragmentshader);

		float pos2[] = 
		{ -0.8,0.8,0.0, 0.8 ,0.0 ,0.6 ,1.0,
		0.8,-0.8,0.0,   0.1 ,0.6 ,0.9 ,1.0,
		-0.8,-0.80,0.0, 0.8 ,0.7 ,0.0 ,1.0,
		0.8,0.8,0.0,    0.2 ,0.9 ,0.8 ,1.0, };

		unsigned int index2[] =
		{ 0,1,2,
		0,1,3};

		SquareVA .reset(VertexArray::Create());
		ref<VertexBuffer> SquareBuffer (VertexBuffer::Create(pos2, sizeof(pos2)));
		ref<BufferLayout> bl2(new BufferLayout);
		bl2->push("position", DataType::Float3);
		bl2->push("color", DataType::Float4);
		IndexBuffer* SquareIndex = IndexBuffer::Create(index2, sizeof(index2));
		SquareVA->AddBuffer(bl2, SquareBuffer);
		SquareVA->SetIndexBuffer((ref<IndexBuffer>)SquareIndex);
		
		std::string SolidColorVertShader = R"(
		#version 410 core
		layout (location = 0) in vec3 pos;
		layout (location = 1) in vec4 col;

			out vec4 m_color;
			uniform mat4 m_ProjectionView;
			uniform mat4 m_ModelTransform;
			void main()
			{
				m_color = col;
				gl_Position = m_ProjectionView * m_ModelTransform * vec4(pos ,1.0);
				
			})";

		std::string SolidColorFragmentshader = R"(
		#version 410 core
		layout (location = 0)out vec4 col;
		uniform vec4 m_color;
		void main(){
			col= m_color;
		}
)";

		SolidColorShader = Shader::Create(SolidColorVertShader, SolidColorFragmentshader);

	}
	virtual void OnUpdate(float deltatime) {
		
		if (Input::IsKeyPressed(HZ_KEY_W))
			v3.y += m_movespeed*deltatime;
		if (Input::IsKeyPressed(HZ_KEY_S))
			v3.y -= m_movespeed*deltatime;
		if (Input::IsKeyPressed(HZ_KEY_A))
			v3.x -= m_movespeed*deltatime;
		if (Input::IsKeyPressed(HZ_KEY_D))
			v3.x += m_movespeed*deltatime;
		if (Input::IsKeyPressed(HZ_KEY_E))
			r += 60 * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_Q))
			r -= 60 * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_UP))
			position.y += ObjSpeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_DOWN))
			position.y -= ObjSpeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_LEFT))
			position.x -= ObjSpeed * deltatime;
		if (Input::IsKeyPressed(HZ_KEY_RIGHT))
			position.x += ObjSpeed * deltatime;

		RenderCommand::ClearColor(glm::vec4(0.8, 0.8, 0.8, 0.8));
		RenderCommand::Clear();

		m_camera.SetPosition(v3);
		m_camera.SetRotation(r);

		//glm::mat4 ModelTransform2 = glm::translate(glm::mat4(1), position); // Set the model transform for now there is no scale or rotation
																			//just multiply the scale and rotation matrix with position to get the full transform
		Renderer::BeginScene(m_camera);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				glm::vec3 tmp = { i, j, 0 };

				glm::mat4 ModelTransform = glm::translate(glm::mat4(1), position+tmp) * glm::scale(glm::mat4(1), glm::vec3(0.3));
				if(j%2==0)
				 SolidColorShader->UpladUniformFloat4("m_color", Color2);
				else
				 SolidColorShader->UpladUniformFloat4("m_color", Color1);
				Renderer::Submit(*SolidColorShader, *SquareVA , ModelTransform);
			}
		}
		Renderer::Submit(*shader, *vao);
		Renderer::EndScene();
	}

	void OnImGuiRender() {
		ImGui::Begin("Color Picker");
		ImGui::ColorPicker4("Color", glm::value_ptr(Color1));
		ImGui::ColorPicker4("Color2", glm::value_ptr(Color2));
		ImGui::End();
	}

	virtual void OnEvent(Hazel::Event& e) {
	}

public:
	glm::vec3 v3={0,0,0};
	float m_movespeed = 20;
	float r = 0;
private:
	bool m_Running = true;

	Shader* shader;
	Shader* SolidColorShader;

	OrthographicCamera m_camera;

	ref<VertexArray> vao;
	ref<VertexArray> SquareVA;

	glm::vec3 position = {0,0,0};
	glm::vec4 Color1;
	glm::vec4 Color2;

	float ObjSpeed = 20;
};

class Sandbox :public Hazel::Application
{
public:
	Sandbox(){
		PushLayer(new GameLayer());
		//PushOverlay(new Hazel::ImGuiLayer());
	}
	~Sandbox(){}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
