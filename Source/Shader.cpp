#include "Shader.h"

std::string get_file_contents(const char* filename) {
	std::ifstream in(filename, std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

void Shader::set_vshader(std::string s){
#ifdef DEBUG_SHADERS
	std::cout << "------------------------------" << std::endl;
	std::cout << "set_vshader:" << std::endl << s << std::endl;
#endif
	const char* vs = s.c_str();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vs, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors(vertexShader, "VERTEX");
	this->vid = vertexShader;
}

void Shader::set_fshader(std::string s){
#ifdef DEBUG_SHADERS
	std::cout << "------------------------------" << std::endl;
	std::cout << "set_fshader:" << std::endl << s << std::endl;
#endif
	const char* fs = s.c_str();
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fs, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");
	this->fid = fragmentShader;
}
void Shader::compile(){
	ID = glCreateProgram();
	glAttachShader(ID, this->vid);
	glAttachShader(ID, this->fid);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
}

Shader::Shader(std::string vertexFile, std::string fragmentFile) {

	std::string vertPath = ("shaders/" + vertexFile);
	std::string fragPath = ("shaders/" + fragmentFile);

	const char* vertSrc = vertPath.c_str();
	const char* fragSrc = fragPath.c_str();

	std::string vertexCode = get_file_contents(vertSrc);
	std::string fragmentCode = get_file_contents(fragSrc);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	std::cout << ID << std::endl;
	std::cout << vertPath << std::endl;
	std::cout << fragPath << std::endl;

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

Shader::Shader(std::string vertexFile, std::string fragmentFile, std::string geomFile) {

	std::string vertPath = ("shaders/" + vertexFile);
	std::string fragPath = ("shaders/" + fragmentFile);
	std::string geomPath = ("shaders/" + geomFile);

	const char* vertSrc = vertPath.c_str();
	const char* fragSrc = fragPath.c_str();
	const char* geomSrc = geomPath.c_str();

	std::string vertexCode = get_file_contents(vertSrc);
	std::string fragmentCode = get_file_contents(fragSrc);
	std::string geomCode = get_file_contents(geomSrc);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();
	const char* geomSource = geomCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");

	GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geomShader, 1, &geomSource, NULL);
	glCompileShader(geomShader);
	checkCompileErrors(geomShader, "GEOMETRY");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glAttachShader(ID, geomShader);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geomShader);

}

void Shader::Activate() {
	glUseProgram(ID);
}

void Shader::Delete() {
	glDeleteProgram(ID);
}
