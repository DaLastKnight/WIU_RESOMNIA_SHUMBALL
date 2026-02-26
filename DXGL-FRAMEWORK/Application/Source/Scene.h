#ifndef SCENE_H
#define SCENE_H

class Scene
{
public:
    Scene() {}
    virtual ~Scene() {}

    // Scene lifecycle
    virtual void Enter() = 0;        // replaces Init()
    virtual void Update(double dt) = 0;
    virtual void Render() = 0;
    virtual void Exit() = 0;

    // Optional
    virtual void Pause() {}
    virtual void Resume() {}
};

#endif