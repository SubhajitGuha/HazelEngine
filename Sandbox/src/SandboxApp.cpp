#include <Hazel.h>
using namespace Hazel;

//This is our game layer
class GameLayer :public Hazel::Layer {
public:
	GameLayer()
		:Layer("Hazel_Layer"), m_camera(-3,3,-3,3)//left right bottom top
	{
		//Single Square 
		float pos[] =
		{0.5,0.5,0.0,   1.0,1.0,
		0.5,-0.5,0.0,   1.0,0.0,
		-0.5,-0.5,0.0,  0.0,0.0,
		-0.5,0.5,0.0,   0.0,1.0};
	
		unsigned int index[] =
		{0,1,2,
		0,2,3};

		
		vao=(VertexArray::Create());//vertex array
		ref<VertexBuffer> vb(VertexBuffer::Create(pos, sizeof(pos) ));//vertex buffer
		ref<BufferLayout> bl=std::make_shared<BufferLayout>(); //buffer layout
		bl->push("position", DataType::Float3);
		bl->push("color", DataType::Float2);
		ref<IndexBuffer> ib(IndexBuffer::Create(index, sizeof(index)));
		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		shader.reset(Shader::Create("Assets/Shaders/TextureShader.glsl"));

		tex2 = Texture2D::Create("Assets/Textures/rickshaw.png");
		texture = Texture2D::Create("Assets/Textures/Test.png");
		
		shader->UploadUniformInt("u_Texture", 0);

		float pos2[] = 
		{ -0.8,0.8,0.0, 0.8 ,0.0 ,0.6 ,1.0,
		0.8,-0.8,0.0,   0.1 ,0.6 ,0.9 ,1.0,
		-0.8,-0.80,0.0, 0.8 ,0.7 ,0.0 ,1.0,
		0.8,0.8,0.0,    0.2 ,0.9 ,0.8 ,1.0, };

		unsigned int index2[] =
		{ 0,1,2,
		0,1,3};

		//For tiled Squares
		SquareVA .reset(VertexArray::Create());
		ref<VertexBuffer> SquareBuffer (VertexBuffer::Create(pos2, sizeof(pos2)));
		ref<BufferLayout> bl2(new BufferLayout);
		bl2->push("position", DataType::Float3);
		bl2->push("color", DataType::Float4);
		IndexBuffer* SquareIndex = IndexBuffer::Create(index2, sizeof(index2));
		SquareVA->AddBuffer(bl2, SquareBuffer);
		SquareVA->SetIndexBuffer((ref<IndexBuffer>)SquareIndex);
		
		SolidColorShader = Shader::Create("Assets/Shaders/SolidColorShader.glsl");

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

		RenderCommand::ClearColor(glm::vec4(0.4, 0.8, 0.8, 0.8));
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
		glm::mat4 ModelTransform2 = glm::scale(glm::mat4(1), glm::vec3(3));
		tex2->Bind(0);
		Renderer::Submit(*shader, *vao, ModelTransform2);
		texture->Bind(0);
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

	std::shared_ptr<Shader> shader,*shader2;
	Shader* SolidColorShader;

	OrthographicCamera m_camera;

	VertexArray* vao;
	ref<VertexArray> SquareVA;

	glm::vec3 position = {0,0,0};
	glm::vec4 Color1 = {1,1,1,1};
	glm::vec4 Color2 = {1,1,1,1};

	float ObjSpeed = 20;
	ref <Texture2D> texture,tex2;
	
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
