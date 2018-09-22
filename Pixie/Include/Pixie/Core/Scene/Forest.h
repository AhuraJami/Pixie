#ifndef PIXIE_CORE_SCENE_FOREST_H
#define PIXIE_CORE_SCENE_FOREST_H

#include <vector>
#include <list>
#include <stack>
#include <sstream>
#include <functional>
#include <memory>

#include "Pixie/Concepts/Object.h"
#include "Pixie/Concepts/Tickable.h"
#include "Pixie/Concepts/PObject.h"
#include "Tree.h"

namespace pixie
{

/**
 * An exclusive scene data structure that holds all the scene entities and
 * their respective components. Each group of entities and components are
 * stored in the form of a tree, which stores them based on their level of
 * dependency on their outer object(See "Tree.h").
 *
 * Responsibilities:
 * - Construct the entity and all its components
 * - Create a tree using entity object as the root
 * - Complete the tree using the construction dependencies of the objects
 * - Keep the trees sorted based on their Tick execution group
 *
 * Usage:
 * 1. Call CreateObject when creating the entity (usually an agent
 *    that will be put directly into the scene.
 * 2. Call CreateComponent for creation of an entity's component as well
 *    as the components of the component.
 */
class Forest
{
public:
	/** Default constructor */
	Forest() = default;

	/**
	 * Constructs a new tree with the Scene entity of type T assigned to
	 * its root
	 * @tparam T (Required) Type of the object that is being created and
	 * assigned to the root of the new tree
	 * @return A pointer to the newly created object T
	 * @warning Do NOT delete this pointer
	 */
	template<class T>
	T* ConstructEntity()
	{
		using std::get;
		using std::move;

		// Initialize a new tree for this object and it's components
		int tick_group = static_cast<int>(trees.size());
		auto& tree = trees.emplace_back(Tree(tick_group));

		// Create the object T
		PObject obj;
		obj.Create<T>();

		// Unless the concept object itself is copied, this pointer
		// remains valid because the object T is held
		// by a unique ptr within the pixie concept object
		T* ptr = obj.StaticCast<T>();

		// Move the object to the root of newly initialized tree
		tree.AddRoot(move(obj));

		// All components of the T should have been initialized and stored
		// by now. Move them from temporary buffers to grow the tree
		PopulateTree(&tree);

		// The dependency tree is formed and all objects are stored in
		// this tree. Clear the temporary buffers and return the pointer
		// to entity object T
		ClearBuffers();

		return ptr;
	}

	/**
	 * Construct an object of type T and stores it within the input PObject
	 * @tparam T (Required) Type of the component that is being created
	 * @param A pointer to an empty PObject
	 */
	template<class T>
	void ConstructPObject(PObject* pobject)
	{
		// See comments of ConstructComponent
		++component_level;

		PObject obj;
		(*pobject).Create<T>();

		temp_buffer.emplace_back(pobject, component_level);

		--component_level;
	}

	/**
	 * Constructs a component of type T and stores it in a temporary buffer
	 * @tparam T (Required) Type of the component that is being created
	 * @return A pointer to the created object of type T
	 * @warning Do NOT delete this pointer
	 */
	template<class T>
	T* ConstructComponent()
	{
		// A component is being created. increments the construction level
		// to keep track of the hierarchy of this sub-component
		++component_level;

		// TODO(Ahura): We can put a guard (like a counter that has a set
		// limit) to prevent circular object construction. For example if
		// level >= 20, it means we've gone 20 levels deep in dependency
		// construction which is something you don't see everyday... We
		// can then stop the construction here, return a nullptr and warn
		// the user. But first check if compiler itself can identify this
		// circular construction. Also find a reasonable value for the
		// limit.
//		if (component_level > 50)
//		{
//			throw std::runtime_error("Circular object creation detected.");
//		}

		PObject obj;
		obj.Create<T>();

		// the pointer remains valid because the object T is held
		// by a unique ptr within one of the pixie's concepts
		T* ptr = obj.StaticCast<T>();

		temp_buffer.emplace_back(std::move(obj), component_level);

		// The component is fully constructed. We are now recursing back
		// to the main object, hence we should decrement the level tracker
		--component_level;

		return ptr;
	}

	/**
	 * Calls Begin for each execution group
	 */
	void CallBegin()
	{
		for (auto& tree : trees)
			tree.CallBegin();
	}

	/**
	 * Calls Tick for each execution group
	 */
	inline void CallTick()
	{
		for (auto& tree : trees)
			tree.CallTick();
	}

	/**
	 * Calls Tick for each execution group
	 */
	void CallEnd()
	{
		for (auto& tree : trees)
			tree.CallEnd();
	}

private:
	/**
	 * Populates the tree using the root object components stored in a
	 * temporary buffer
	 * @param [in] tree The tree that will hold all this group of components
	 */
	void PopulateTree(Tree* tree)
	{
		/*
		 * The following algorithm and overall the whole idea of using
		 * this forest to form the construction trees is as follows:
		 * Imagine the following scenario where the outer Agent entity
		 * creates components C1 and C5 in its constructor. Components
		 * C1 and C5 in turn create other sub-components in their
		 * constructors and some of these sub-components may create
		 * their own sub-components and, you get the idea. This
		 * construction dependency is shown in the following graph:
		 *
		 * Agent -> C1 -> C2
		 * 	   |     |
		 * 	   |	 -> C3 -> C4
		 * 	   |
		 * 	   -> C5 -> C6
		 *
		 * With such dependency, C2 should be the first one to be fully
		 * constructed, followed by C4, C3, C1, C6, C5, and finally the
		 * Agent itself. Since the objects are created independent of
		 * each other (i.e. we are not directly passing the identity of
		 * the class who is constructing the component (outer class) as a
		 * separate argument when calling CreateComponent), we use another
		 * object as to observe these constructions and record the
		 * dependencies. This is exactly what Forest class does here. When
		 * the entity object (which is usually expected to be put directly
		 * into the scene) is queued for construction, the forest first
		 * creates a brand new tree and only then calls the constructor of
		 * this scene object. As the constructed of this object is being
		 * executed, pixie starts the construction of its dependencies
		 * with a call to CreateComponent for each of its dependent
		 * component. Our forest observes these component constructions
		 * and keeps track of two states: the beginning of a component
		 * construction and the end of its construction. At the beginning
		 * of the construction of a component, forest increments the tree
		 * level tracker as we have moved one level deep into the
		 * construction. It then calls the constructor of that component
		 * and waits for the component to be fully initialized. This
		 * component might also create additional sub-components for which
		 * the above process is repeated and the level tracker will keep
		 * track of how deep in construction we've gone. Once the
		 * construction is finished and the component is ready, we add it
		 * to a temporary buffer while also recording its level. We then
		 * decrement the level tracker, and recurse back all the way to
		 * the top of the construction hierarchy and to the entity object
		 * that started all of this. At this point, the entity object
		 * construction is over and the object is ready. We add this
		 * object to the root of the tree, and attempt to unwrap the
		 * temporary buffer and form the tree using all the dependent
		 * components. This last part is where this function 'PopulateTree'
		 * comes into play.
		 * For the graph shown above, the temporary buffer looks as follows:
		 *
		 * [C2, C4, C3, C1, C6, C5] <- temp_buffer.fist
		 * [ 2,  3,  2,  1,  2,  1] <- temp_buffer.second
		 *
		 * where temp_buffer.first holds the object itself (stored in
		 * one of the Pixie's concepts), and the temp_buffer.second
		 * corresponds to their level in the construction tree.
		 *
		 * Note that with this approach, the following condition is
		 * guaranteed to be true:
		 *
		 * 		abs(L_i - L_(i+1)) <= 1
		 *
		 * where L_i, L_(i+1) - level of component i and component i+1
		 *
		 * With this fact, and knowing that Agent is at root (level=0) we
		 * can use these information and grow our tree.
		 *
		 * Time Complexity O(N)
		 */
		using namespace std;

		// A stack to keep track of outer object (parent in tree terms)
		stack<Tree::Node*> parents;

		// first parent is always the root of the tree
		parents.push(&tree->root);

		while(not temp_buffer.empty())
		{
			auto& object = temp_buffer.back().first;
			auto level = temp_buffer.back().second;
			auto* parent = parents.top();

			// if the last object in temp_buffer is one level
			// higher than the parent, then this is indeed the
			// correct parent of the object.
			if(not parents.empty() and level > parent->level)
			{
				// Move the object to the tree
				// We're going one level deeper to identify the children
				// of this newly added node
				auto& node = tree->AddNode(move(object), parent);
				parents.push(&node);

				// We just moved the object to the tree. Hence we can
				// safely discard the last element of the temp_buffer.
				temp_buffer.pop_back();
			}
			else if (not parents.empty()) // and level <= parent->level)
			{ /* Unnecessary check but never hurts to be safe :) */
				// Our object is either on the same level with this
				// parent or is one level lower, hence all children
				// of this object are accounted for. We can pop this
				// parent out of stack and start dealing with children
				// of the grandparent :)
				parents.pop();
			}
			else
			{
				// must not reach here
				std::ostringstream ss;
				ss
						<< "Failed building the construction dependency tree.\n"
						<< "buffer size: " << temp_buffer.size() << "\n"
						<< "stack size: "  << parents.size() << "\n";

				throw std::runtime_error(ss.str());
			}
		}

		tree->ReverseContainers();
	}

	/**
	 * Clears the temporary buffers and resets the index of the
	 * construction level
	 */
	void ClearBuffers()
	{
		temp_buffer.clear();
		component_level = 0;
	}

	/// Vector of trees sorted by their tick group
	std::deque<Tree> trees;

	/// Tracks the construction level of the object and its components
	int component_level = 0;

	/// Temporary buffer that holds components of the Outer object

#ifdef __APPLE_CLANG__
	std::deque<std::pair<mpark::variant<PObject, PObject*>, int>> temp_buffer;
#else
	std::deque<std::pair<std::variant<PObject, PObject*>, int>> temp_buffer;
#endif
};

}

#endif //PIXIE_CORE_SCENE_FOREST_H
