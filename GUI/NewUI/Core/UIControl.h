#pragma once
#include <list>
#include <algorithm>

class UIControl
{
  public:
    virtual void Show() = 0;

    virtual bool Focused() = 0;

    // Events
    virtual bool OnKey(int key)                  {return DefaultOnKey(key);}
    virtual bool OnMouseLeftClick(int x, int y)  {return DefaultOnMouseLeftClick(x, y);}
    virtual bool OnMouseRightClick(int x, int y) {return DefaultOnMouseRightClick(x, y);}
    virtual bool OnMouseEnter()                  {return DefaultOnMouseEnter();}
    virtual bool OnMouseLeave()                  {return DefaultOnMouseLeave();}

    // Container
    virtual bool AddControl(UIControl *ctrl)
    {
      if (std::find(children.begin(), children.end(), ctrl) == children.end())
      {
        children.push_back(ctrl);
        ctrl->parent = this;
        return true;
      }
      return false;
    }

    virtual bool RemoveControl(UIControl *ctrl)
    {
      auto i = std::find(children.begin(), children.end(), ctrl);

      children.remove(ctrl);
      if (i != children.end())
      {
        ctrl->parent = nullptr;
        return true;
      }
      return false;
    }

  protected:
    UIControl              *parent;
    std::list<UIControl *>  children;


    bool DefaultOnKey(int key)
    {
      for (auto i = children.begin(); i != children.end(); ++i)
        if ((*i)->OnKey(key))
          return true;
      return false;
    }

    bool DefaultOnMouseLeftClick(int x, int y)
    {
      for (auto i = children.begin(); i != children.end(); ++i)
        if ((*i)->OnMouseLeftClick(x, y))
          return true;
      return false;
    }

    bool DefaultOnMouseRightClick(int x, int y)
    {
      for (auto i = children.begin(); i != children.end(); ++i)
        if ((*i)->OnMouseRightClick(x, y))
          return true;
      return false;
    }

    bool DefaultOnMouseEnter()
    {
      for (auto i = children.begin(); i != children.end(); ++i)
        if ((*i)->OnMouseEnter())
          return true;
      return false;
    }

    bool DefaultOnMouseLeave()
    {
      for (auto i = children.begin(); i != children.end(); ++i)
        if ((*i)->OnMouseLeave())
          return true;
      return false;
    }
};