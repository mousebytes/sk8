#ifndef _LEVELEDITOR_H
#define _LEVELEDITOR_H

#include "_common.h"
#include "_StaticModelInstance.h"
#include "_Button.h"
#include "_camera.h"
#include"_CubeHitbox.h"
#include"_SphereHitbox.h"
#include "_AnimatedModelInstance.h"

class _LevelEditor {
public:
    _LevelEditor();
    ~_LevelEditor();

    void Init(int width, int height);
    void Update(HWND hWnd, _camera* cam);
    void Draw();

    void SetReferenceBlueprint(_AnimatedModel* bp);
    
    void HandleKeyInput(WPARAM wParam);
    
    // Updated to handle Right Clicks (removing items)
    bool HandleMouseClick(UINT uMsg, int mouseX, int mouseY);

    void SaveLevel(string filename);
    void LoadLevel(string filename);
    void ClearLevel();

private:
    vector<_StaticModelInstance*> m_placedObjects;
    map<string, _StaticModel*> m_blueprints;
    string m_selectedType;
    
    _StaticModelInstance* m_ghostObject;
    _StaticModelInstance* m_hoveredObject; // The object currently under the mouse
    _StaticModelInstance* m_floorPlane;    // The huge area for the player to stand on
    _AnimatedModelInstance* m_refPlayer;

    vector<_Button*> m_itemButtons; 
    vector<string> m_itemNames;     
    
    _Button* m_saveButton;
    _Button* m_loadButton;
    _Button* m_exitButton;
    
    float m_gridSize;
    float m_currentRotation; 
    Vector3 m_cursorPos;     

    Vector3 GetWorldPosFromMouse(HWND hWnd, _camera* cam);
    void SetGhost(string type);
    void ScaleGhost(float amount);
    
    // Helper to check if mouse ray hits an object
    _StaticModelInstance* RaycastCheck(HWND hWnd, _camera* cam);
};

#endif