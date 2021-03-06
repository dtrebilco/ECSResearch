#pragma once

#include <ECS.h>
#include "../Utils.h"

class Bounds : public ComponentManager
{
public:

  class Component : public ComponentBase<Bounds>
  {
  public:
    inline vec3& GetCenter() { return m_manager->m_centers[m_index]; }
    inline void SetCenter(const vec3& i_newData) { m_manager->m_centers[m_index] = i_newData; }

    inline vec3& GetExtents() { return m_manager->m_extents[m_index]; }
    inline void SetExtents(const vec3& i_newData) { m_manager->m_extents[m_index] = i_newData; }
  };

  inline void OnComponentAdd(EntityID i_entity, uint16_t i_index)
  {
    m_centers.insert(m_centers.begin() + i_index, vec3(0.0f));
    m_extents.insert(m_extents.begin() + i_index, vec3(0.0f));
  }

  void OnComponentRemove(EntityID i_entity, uint16_t i_index) override
  {
    m_centers.erase(m_centers.begin() + i_index);
    m_extents.erase(m_extents.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_centers.reserve(i_count);
    m_extents.reserve(i_count);
  }

  std::vector<vec3> m_centers; //!< The centers
  std::vector<vec3> m_extents; //!< The extents

};


class WorldBounds : public ComponentManager
{
public:

  class Component : public ComponentBase<WorldBounds>
  {
  public:
    inline vec3& GetCenter() { return m_manager->m_centers[m_index]; }
    inline void SetCenter(const vec3& i_newData) { m_manager->m_centers[m_index] = i_newData; }

    inline vec3& GetExtents() { return m_manager->m_extents[m_index]; }
    inline void SetExtents(const vec3& i_newData) { m_manager->m_extents[m_index] = i_newData; }
  };

  inline void OnComponentAdd(EntityID i_entity, uint16_t i_index)
  {
    m_centers.insert(m_centers.begin() + i_index, vec3(0.0f));
    m_extents.insert(m_extents.begin() + i_index, vec3(0.0f));
  }

  void OnComponentRemove(EntityID i_entity, uint16_t i_index) override
  {
    m_centers.erase(m_centers.begin() + i_index);
    m_extents.erase(m_extents.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_centers.reserve(i_count);
    m_extents.reserve(i_count);
  }

  std::vector<vec3> m_centers; //!< The centers
  std::vector<vec3> m_extents; //!< The extents

};

//class BoundsSIMD : public ComponentManager
//{
//public:
//
//  class Component : public ComponentBase<BoundsSIMD>
//  {
//  public:
//    inline vec3 GetCenter()
//    {
//      return vec3(m_manager->m_centerX[m_index],
//                  m_manager->m_centerY[m_index],
//                  m_manager->m_centerZ[m_index]);
//    }
//
//    inline void SetCenter(const vec3& i_newData)
//    {
//      m_manager->m_centerX[m_index] = i_newData.x;
//      m_manager->m_centerY[m_index] = i_newData.y;
//      m_manager->m_centerZ[m_index] = i_newData.z;
//    }
//
//    inline vec3 GetExtents()
//    {
//      return vec3(m_manager->m_extentX[m_index],
//                  m_manager->m_extentY[m_index],
//                  m_manager->m_extentZ[m_index]);
//    }
//
//    inline void SetExtents(const vec3& i_newData)
//    {
//      m_manager->m_extentX[m_index] = i_newData.x;
//      m_manager->m_extentY[m_index] = i_newData.y;
//      m_manager->m_extentZ[m_index] = i_newData.z;
//    }
//
//  };
//
//  BoundsSIMD()
//  {
//    // Add padding to the end of the storage arrays
//    for (uint32_t i = 0; i < 3; i++)
//    {
//      m_centerX.push_back(0.0f);
//      m_centerY.push_back(0.0f);
//      m_centerZ.push_back(0.0f);
//
//      m_extentX.push_back(0.0f);
//      m_extentY.push_back(0.0f);
//      m_extentZ.push_back(0.0f);
//    }
//  }
//
//  inline void OnComponentAdd(EntityID i_entity, uint16_t i_index)
//  {
//    m_centerX.insert(m_centerX.begin() + i_index, 0.0f);
//    m_centerY.insert(m_centerY.begin() + i_index, 0.0f);
//    m_centerZ.insert(m_centerZ.begin() + i_index, 0.0f);
//
//    m_extentX.insert(m_extentX.begin() + i_index, 0.0f);
//    m_extentY.insert(m_extentY.begin() + i_index, 0.0f);
//    m_extentZ.insert(m_extentZ.begin() + i_index, 0.0f);
//  }
//
//  void OnComponentRemove(EntityID i_entity, uint16_t i_index) override
//  {
//    m_centerX.erase(m_centerX.begin() + i_index);
//    m_centerY.erase(m_centerY.begin() + i_index);
//    m_centerZ.erase(m_centerZ.begin() + i_index);
//
//    m_extentX.erase(m_extentX.begin() + i_index);
//    m_extentY.erase(m_extentY.begin() + i_index);
//    m_extentZ.erase(m_extentZ.begin() + i_index);
//  }
//
//  inline void ReserveComponent(uint16_t i_count)
//  {
//    m_centerX.reserve(i_count + 3);
//    m_centerY.reserve(i_count + 3);
//    m_centerZ.reserve(i_count + 3);
//
//    m_extentX.reserve(i_count + 3);
//    m_extentY.reserve(i_count + 3);
//    m_extentZ.reserve(i_count + 3);
//  }
//
//  std::vector<float> m_centerX; //!< The center X
//  std::vector<float> m_centerY; //!< The center Y
//  std::vector<float> m_centerZ; //!< The center Z
//
//  std::vector<float> m_extentX; //!< The extents X
//  std::vector<float> m_extentY; //!< The extents Y
//  std::vector<float> m_extentZ; //!< The extents Z
//
//};

