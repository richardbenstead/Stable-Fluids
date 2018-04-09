#include "Quad.hpp"

Quad::Quad(GLfloat size)
{
    GLfloat low = -size / 2.0f;
    GLfloat high = size / 2.0f;
    GLfloat vertices[3 * 3] =
        {
            low, low, low,
            low, high, low,
            high, high, low};
    //GLuint indecise[ELEMENT_SIZE] = {0, 1, 2};
    //VAO（バッファデータ・設定群)の生成
    glGenVertexArrays(1, &vaoID);
    //現在のVAOに設定
    glBindVertexArray(vaoID);
    //VBO（バッファ）の生成
    glGenBuffers(1, &vboID);
    //現在のバッファに設定
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    //現在のバッファにデータを送信
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //データ型の設定(float * 3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    //シェーダで0番地を使用
    glEnableVertexAttribArray(0);
    //インデックスの設定
    glGenBuffers(1, &index_bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_bufferID);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecise), indecise, GL_STATIC_DRAW);
    //カラーバッファの生成
    glGenBuffers(1, &color_bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, color_bufferID);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    //VAO設定を終了する
    glBindVertexArray(0);
}
Quad::~Quad()
{
    glDeleteBuffers(1, &index_bufferID);
    glDeleteBuffers(1, &vboID);
    glDeleteBuffers(1, &color_bufferID);
    glDeleteVertexArrays(1, &vaoID);
}

void Quad::Draw()
{
    //VAOの有効化（VAOに割り当てた設定とバッファが復元される)
    glBindVertexArray(vaoID);
    //インデックスを用いて描画する
    glDrawElements(GL_TRIANGLES, ELEMENT_SIZE, GL_UNSIGNED_INT, nullptr);
    //VAOの無効化
    glBindVertexArray(0);
}