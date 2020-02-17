//
// Created by 赵帆 on 2020-02-15.
//

#include <GLES2/gl2.h>
#include "XShader.h"
#include "XLog.h"

// 顶点着色器glsl
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute
        vec4 aPosition; // 顶点坐标
        attribute
        vec2 aTexCoord; // 材质顶点坐标
        varying
        vec2 vTexCoord; // 输出的材质坐标
        void main() {
            vTexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
            gl_Position = aPosition;
        }
);

// 片元着色器，软解码和部分x86硬解码
static const char *fragYUV420P = GET_STR(
        precision
        mediump float; // 进度
        varying
        vec2 vTexCoord; // 顶点着色器传递过来的坐标
        uniform
        sampler2D yTexture; // 输入的材质，灰度图
        uniform
        sampler2D uTexture; // 输入的材质
        uniform
        sampler2D vTexture; // 输入的材质
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
            rgb = mat3(
                    1.0, 1.0, 1.0,
                    0.0, -0.39465, 2.03211,
                    1.13983, -0.5806, 0.0) * yuv;
            // 输出像素颜色
            gl_FragColor = vec4(rgb, 1.0);
        }
);

//片元着色器,软解码和部分x86硬解码
static const char *fragNV12 = GET_STR(
        precision
        mediump float;    //精度
        varying
        vec2 vTexCoord;     //顶点着色器传递的坐标
        uniform
        sampler2D yTexture; //输入的材质（不透明灰度，单像素）
        uniform
        sampler2D uvTexture;
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uvTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(uvTexture, vTexCoord).a - 0.5;
            rgb = mat3(1.0, 1.0, 1.0,
                       0.0, -0.39465, 2.03211,
                       1.13983, -0.58060, 0.0) * yuv;
            //输出像素颜色
            gl_FragColor = vec4(rgb, 1.0);
        }
);

//片元着色器,软解码和部分x86硬解码
static const char *fragNV21 = GET_STR(
        precision
        mediump float;    //精度
        varying
        vec2 vTexCoord;     //顶点着色器传递的坐标
        uniform
        sampler2D yTexture; //输入的材质（不透明灰度，单像素）
        uniform
        sampler2D uvTexture;
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uvTexture, vTexCoord).a - 0.5;
            yuv.b = texture2D(uvTexture, vTexCoord).r - 0.5;
            rgb = mat3(1.0, 1.0, 1.0,
                       0.0, -0.39465, 2.03211,
                       1.13983, -0.58060, 0.0) * yuv;
            //输出像素颜色
            gl_FragColor = vec4(rgb, 1.0);
        }
);

static GLuint initShader(const char *code, GLint type) {
    GLuint sh = glCreateShader(type);
    if (sh == 0) {
        XLOGE("glCreateShader %d failed!", type);
        return 0;
    }
    // 加载 shader
    glShaderSource(sh,
                   1,      // shader 数量
                   &code,  // shader 代码
                   0);     // 代码长度
    // 编译 shader
    glCompileShader(sh);
    // 获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        XLOGE("glCompileShader failed!");
        return 0;
    } else {
        XLOGE("glCompileShader %d success!", type);
    }
    return sh;
}

bool XShader::init(XShaderType type) {
    // 顶点和片元 shader 初始化
    vsh = initShader(vertexShader, GL_VERTEX_SHADER);
    if (vsh == 0) {
        XLOGE("init shader GL_VERTEX_SHADER failed");
        return false;
    }
    XLOGE("init shader GL_VERTEX_SHADER success");
    switch (type) {
        case XSHADER_YUV420P:
            fsh = initShader(fragYUV420P, GL_FRAGMENT_SHADER);
            break;
        case XSHADER_NV12:
            fsh = initShader(fragNV12, GL_FRAGMENT_SHADER);
            break;
        case XSHADER_NV21:
            fsh = initShader(fragNV21, GL_FRAGMENT_SHADER);
            break;
        default:
            XLOGE("XShader format is error");
            return false;
    }
    if (fsh == 0) {
        XLOGE("init shader GL_FRAGMENT_SHADER failed");
        return false;
    }
    XLOGE("init shader GL_FRAGMENT_SHADER success");

    // 创建渲染程序
    program = glCreateProgram();
    // 向渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);
    // 链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        XLOGE("glLinkProgram failed");
        return false;
    }

    // 着色器激活
    glUseProgram(program);
    XLOGE("glUseProgram success");

    // 加入三维顶点数据,两个三角形组成正方形
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
    };
    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    // 传递数据
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, ver);

    // 加入材质坐标数据
    static float txt[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint atex = static_cast<GLuint>(glGetAttribLocation(program, "aTexCoord"));
    glEnableVertexAttribArray(atex);
    // 传递数据
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txt);
    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); //对于纹理第1层
    switch (type) {
        case XSHADER_YUV420P:
            glUniform1i(glGetUniformLocation(program, "uTexture"), 1); //对于纹理第2层
            glUniform1i(glGetUniformLocation(program, "vTexture"), 2); //对于纹理第3层
            break;
        case XSHADER_NV12:
        case XSHADER_NV21:
            glUniform1i(glGetUniformLocation(program, "uvTexture"), 1); //对于纹理第2层
            break;
    }
    XLOGE("初始化Shader成功！");
    return true;
}

void XShader::getTexture(unsigned int index, int width, int height, unsigned char *buffer,
                         bool isAlpha) {
    unsigned int format = GL_LUMINANCE;
    if (isAlpha) format = GL_LUMINANCE_ALPHA;
    if (textures[index] == 0) {
        // 1 表示一次只创建一个
        glGenTextures(1, &textures[index]);
        // 设置纹理属性
        glBindTexture(GL_TEXTURE_2D, textures[index]);
        // 缩小和放大的过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置纹理格式和大小
        glTexImage2D(GL_TEXTURE_2D, 0,
                     format, // gpu 内部格式，灰度图
                     width, // 尺寸必须是2的次方
                     height,
                     0,// 边框
                     format, // 数据像素格式与上面一致
                     GL_UNSIGNED_BYTE, // 像素的数据类型
                     nullptr // 纹理的数据
        );
    }

    glActiveTexture(GL_TEXTURE0 + index); // 激活当前 index 层纹理
    glBindTexture(GL_TEXTURE_2D, textures[index]); // 绑定到创建的纹理中
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE,
                    buffer); // 替换纹理内容
}

void XShader::draw() {
    if (!program) return;
    //三维绘制存放的顶点信息
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
