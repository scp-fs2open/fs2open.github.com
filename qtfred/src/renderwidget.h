#pragma once
#include <memory>
#include <unordered_map>
#include <QGLWidget>

namespace fso {
namespace fred {

class Editor;

class RenderWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = 0);
    void setEditor(std::shared_ptr<Editor> editor) { fred = editor; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

signals:

private:
    std::unordered_map<int, int> qt2fsKeys;
    std::shared_ptr<Editor> fred;
};

} // namespace fred
} // namespace fso
