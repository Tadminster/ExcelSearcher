#pragma once

template <typename T>
class Singleton
{
    //static
private:
    static T* instance;

public:
    static T* GetInstance()
    {
        if (!instance)
            instance = new T;

        return instance;
    };
    
public:
    virtual ~Singleton() {};
    void DeleteSingleton()
    {
        delete instance;
        instance = nullptr;
    };
};

template <typename T>
T* Singleton<T>::instance = nullptr;
class Scene
{
public:
    virtual ~Scene() {};
    virtual void Init() = 0;
    virtual void Release() = 0;
    virtual void Update() = 0;
    virtual void LateUpdate() = 0;
    virtual void Render() = 0;
    virtual void ResizeScreen() = 0;
};
