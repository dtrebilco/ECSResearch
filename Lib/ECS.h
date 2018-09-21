#pragma once
#include "Common.h"

#include <cstdint>
#include <vector>


enum class GroupID : uint16_t {};  //!< Supports 65k groups
enum class EntitySubID : uint16_t {};  //!< Supports 65k entities per group

struct EntityID
{
  GroupID m_groupID;   //!< Index of the group
  EntitySubID m_subID; //!< Index of the entity in the group
};
static_assert(sizeof(EntityID) == 4, "Unexpected size");

inline bool operator == (EntityID a, EntityID b)
{
  return (a.m_groupID == b.m_groupID) && (a.m_subID == b.m_subID);
}

inline bool operator !=(EntityID a, EntityID b)
{
  return !(a == b);
}

inline bool operator < (EntityID a, EntityID b)
{
  return (a.m_groupID < b.m_groupID) || 
         ((a.m_groupID == b.m_groupID) && (a.m_subID < b.m_subID));
}

const EntityID EntityID_None { GroupID(UINT16_MAX),  EntitySubID(UINT16_MAX) };

class ComponentFlags
{
public:

  inline bool HasComponent(EntitySubID i_entity) const
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    return (m_bitData[index] & mask) != 0;
  }

  inline const std::vector<uint64_t>& GetBits() const { return m_bitData; }

private:

  friend class EntityGroup;
  friend class ComponentManager;
  template<typename T> friend class Context;

  std::vector<uint64_t> m_bitData; //!< The array of bit data

};

class ComponentManager : public ComponentFlags
{
public:

  virtual ~ComponentManager() {}

  inline uint16_t GetComponentIndex(EntitySubID i_entity)
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    return m_prevSum[index] + PopCount64(GetBits()[index] & (mask - 1));
  }

  inline uint16_t GetComponentCount()
  {
    if (m_prevSum.size() == 0)
    {
      return 0;
    }

    // If this gets called a lot, consider adding an extra count entry at the end of m_prevSum?
    return m_prevSum.back() + PopCount64(GetBits().back()); 
  }

  virtual void OnComponentRemove(uint16_t i_index) = 0;

  DebugAccessCheck m_accessCheck; //!< Debug access checker

private:
  friend class EntityGroup;
  template<typename T> friend class Context;

  std::vector<uint16_t> m_prevSum; //!< The sum of all previous bits

  uint16_t SetBit(EntitySubID i_entity);
  uint16_t ClearBit(EntitySubID i_entity);

};

template <typename T, typename E>
T& GetManager(E& i_group);

class FlagManager : public ComponentFlags {};


template<typename T>
class DebugAccessLock
{
public:

  inline DebugAccessLock() {}
  inline DebugAccessLock(const DebugAccessLock& i_copy) noexcept { *this = i_copy; }
  inline DebugAccessLock(DebugAccessLock&& i_other) noexcept
  {
    this->m_type = i_other.m_type;
    i_other.m_type = nullptr;
  }


  inline DebugAccessLock& operator=(const DebugAccessLock& i_copy) noexcept { *this = i_copy.m_type; return *this; }
  inline DebugAccessLock& operator=(DebugAccessLock&& i_other) noexcept
  {
    this->m_type = i_other.m_type;
    i_other.m_type = nullptr;
    return *this;
  }

  inline ~DebugAccessLock()
  {
    if (m_type) { m_type->m_accessCheck.ReleaseLock(); }
  }

  inline DebugAccessLock& operator=(T* i_copy) noexcept
  {
    if (m_type) { m_type->m_accessCheck.ReleaseLock(); }
    m_type = i_copy;
    if (m_type) { m_type->m_accessCheck.AddLock(); }
    return *this;
  }

  inline T* operator->() const { return m_type; }

private:

  T* m_type = nullptr; //!< The type with the counter

};

template<typename T>
class ComponentBase
{
public:

  uint16_t m_index = 0; //!< The index into the manager of the component
  DebugAccessLock<T> m_manager; //!< The manager of the component

};

template<typename T>
class ComponentTypeManager : public ComponentManager
{
public:

  class ComponentType : public ComponentBase<ComponentTypeManager<T>>
  {
  public:

    inline T* operator->() const { return &this->m_manager->m_data[this->m_index]; }

    inline T& GetData()
    {
      return this->m_manager->m_data[this->m_index];
    }
  };

  inline void OnComponentAdd(uint16_t i_index)
  {
    m_data.insert(m_data.begin() + i_index, T());
  }

  inline void OnComponentAdd(uint16_t i_index, const T& i_addData)
  {
    m_data.insert(m_data.begin() + i_index, i_addData);
  }

  void OnComponentRemove(uint16_t i_index) override
  {
    m_data.erase(m_data.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_data.reserve(i_count);
  }

  std::vector<T> m_data; //!< The data stored

};


/// \brief A entity group base class. This is intended to be inherited from and contain ComponentManagers
class EntityGroup
{
public:

  inline bool IsValid(EntitySubID i_entity) const
  {
    return (uint16_t)i_entity < m_entityMax;
  }

  inline uint16_t GetEntityMax() const
  {
    return m_entityMax;
  }

  inline void AddManager(ComponentManager* i_manager)
  {
    AT_ASSERT(m_entityMax == 0);
    m_managers.push_back(i_manager);
  }

  inline void AddManager(FlagManager* i_manager)
  {
    AT_ASSERT(m_entityMax == 0);
    m_flagManagers.push_back(i_manager);
  }

private:
  template<typename T> friend class Context;

  uint16_t m_entityMax = 0;                   //!< Max entity allocated 

  std::vector<ComponentManager*> m_managers;  //!< Registry array of component managers
  std::vector<FlagManager*> m_flagManagers;   //!< Registry array of single flag managers

  std::vector<EntitySubID> m_deletedEntities; //!< Array of re-usable entity ids that have been deleted

  EntitySubID AddEntity();
  void RemoveEntity(EntitySubID i_entity);
  void ReserveEntities(uint16_t i_count);

};


template<class E>
class Context
{
public:

  ~Context()
  {
    for (E* e : m_groups)
    {
      delete e;
    }
    m_groups.clear();
  }

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsValid(GroupID i_group) const
  {
    return ((uint16_t)i_group < m_groups.size()) &&
            (m_groups[(uint16_t)i_group] != nullptr);
  }

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsValid(EntityID i_entity) const
  {
    return IsValid(i_entity.m_groupID) &&
           m_groups[(uint16_t)i_entity.m_groupID]->IsValid(i_entity.m_subID);
  }

  /// \brief Add a new entity group
  /// \return The new group is returned
  inline GroupID AddEntityGroup()
  {
    // Loop and find a vacant index
    for (uint32_t i = 0; i < m_groups.size(); i++)
    {
      // Allocate to that index
      if (m_groups[i] == nullptr)
      {
        m_groups[i] = new E();
        return GroupID(i);
      }
    }

    AT_ASSERT(m_groups.size() <= UINT16_MAX);

    // Add a new item 
    m_groups.push_back(new E());
    return GroupID(m_groups.size() - 1);
  }

  /// \brief Remove an entity group. 
  ///        NOTE: Ensure the group is not being accessed (ie. iterated upon) when doing this.
  /// \param i_group The group ID to remove
  inline virtual void RemoveEntityGroup(GroupID i_group)
  {
    AT_ASSERT(IsValid(i_group));

    delete m_groups[(uint16_t)i_group];
    m_groups[(uint16_t)i_group] = nullptr;
  }

  /// \brief Add an entity to the indicated group
  /// \return The added entity is returned
  inline EntityID AddEntity(GroupID i_group)
  {
    AT_ASSERT(IsValid(i_group));
    return EntityID{ i_group , m_groups[(uint16_t)i_group]->AddEntity() };
  }

  /// \brief Remove the entity from the context
  ///        NOTE: Ensure the entity is not being accessed (ie. iterated upon) when doing this.
  /// \param i_entity The entity to remove.
  inline virtual void RemoveEntity(EntityID i_entity)
  {
    AT_ASSERT(IsValid(i_entity.m_groupID));
    m_groups[(uint16_t)i_entity.m_groupID]->RemoveEntity(i_entity.m_subID);
  }

  template <class T>
  inline bool HasComponent(EntityID i_entity) const
  {
    AT_ASSERT(IsValid(i_entity));
    return GetManager<T>(*m_groups[(uint16_t)i_entity.m_groupID]).HasComponent(i_entity.m_subID);
  }

  template <class T>
  inline typename T::ComponentType GetComponent(EntityID i_entity) const
  {
    AT_ASSERT(HasComponent<T>(i_entity));
    E& group = *m_groups[(uint16_t)i_entity.m_groupID];

    typename T::ComponentType retType;
    retType.m_manager = &GetManager<T>(group);
    retType.m_index = retType.m_manager->GetComponentIndex(i_entity.m_subID);
    return retType;
  }
  
  template <class T, typename... Args>
  inline typename T::ComponentType AddComponent(EntityID i_entity, Args&... args)
  {
    AT_ASSERT(IsValid(i_entity));
    E& group = *m_groups[(uint16_t)i_entity.m_groupID];
    T& manager = GetManager<T>(group);

    // Debug check that there are no active accessors to the data
    manager.m_accessCheck.CheckLock();

    typename T::ComponentType retType;
    retType.m_manager = &manager;
    retType.m_index = manager.SetBit(i_entity.m_subID);
    manager.OnComponentAdd(retType.m_index, args...);
    return retType;
  }

  template <class T>
  inline void RemoveComponent(EntityID i_entity)
  {
    AT_ASSERT(IsValid(i_entity));
    E& group = *m_groups[(uint16_t)i_entity.m_groupID];
    T& manager = GetManager<T>(group);

    // Debug check that there are no active accessors to the data
    manager.m_accessCheck.CheckLock();

    uint16_t index = manager.ClearBit(i_entity.m_subID);
    manager.OnComponentRemove(index);
  }

  inline E* GetGroup(EntityID i_entity)
  {
    AT_ASSERT(IsValid(i_entity));
    E* group = m_groups[(uint16_t)i_entity.m_groupID];
    return group;
  }

  template <class T>
  inline bool HasFlag(EntityID i_entity) const
  {
    AT_ASSERT(IsValid(i_entity));
    FlagManager& manager = GetManager<T>(*m_groups[(uint16_t)i_entity.m_groupID]);
    return manager.HasComponent(i_entity.m_subID);
  }

  template <class T>
  inline void SetFlag(EntityID i_entity, bool i_value)
  {
    AT_ASSERT(IsValid(i_entity));
    E& group = *m_groups[(uint16_t)i_entity.m_groupID];
    FlagManager& manager = GetManager<T>(group);

    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity.m_subID & 0x3F);
    uint16_t index = (uint16_t)i_entity.m_subID >> 6;

    if (i_value)
    {
      manager.m_bitData[index] |= mask;
    }
    else
    {
      manager.m_bitData[index] &= ~mask;
    }
  }

  inline void ReserveGroups(uint16_t i_count)
  {
    m_groups.reserve(i_count);
  }

  inline void ReserveEntities(GroupID i_group, uint16_t i_count)
  {
    AT_ASSERT(IsValid(i_group));
    m_groups[(uint16_t)i_group]->ReserveEntities(i_count);
  }

  template <class T>
  inline void ReserveComponent(GroupID i_group, uint16_t i_count)
  {
    AT_ASSERT(IsValid(i_group));
    E& group = *m_groups[(uint16_t)i_group];
    T& manager = GetManager<T>(group);

    // Debug check that there are no active accessors to the data
    manager.m_accessCheck.CheckLock();

    manager.ReserveComponent(i_count);
  }

  inline const std::vector<E*>& GetGroups() const
  {
    return m_groups;
  }

protected:

  std::vector<E*> m_groups;   //!< Array of entity groups

};

