#ifndef PIXIE_CORE_SCENE_FOREST_H
#define PIXIE_CORE_SCENE_FOREST_H

#include <vector>
#include <stack>
#include <sstream>

#include "Pixie/Concepts/Object.h"
#include "Pixie/Concepts/Tickable.h"

#ifdef __APPLE_CLANG__
#include <mpark/variant.hpp>
#else
#include <variant>
#endif

namespace pixie
{

/**
 * A scene exclusive data structure that holds all the scene objects and
 * their respective components. Each group of object and components are
 * stored in the form of a tree, whom stores them based on their level of
 * dependency on their outer object.
 *
 * Responsibilities:
 * - Construct main object and all its components
 * - Create a tree using main Outer object as the root
 * - Complete the tree using the construction dependencies of the objects
 * - Keep the trees sorted based on their Tick execution group
 *
 * Usage:
 * 1. Call CreateObject when creating the main object (usually an agent
 *    that will be put directly into the scene.
 * 2. Call CreateComponent for creation of a main object's component as well
 *    as the components of the component.
 */
class Forest
{
	/// Supported object types
	template<class... types>
#ifdef __APPLE_CLANG__
	using variant = mpark::variant<types...>;
#else
	using variant = std::variant<types...>;
#endif

	using object_type = variant<Object, Tickable>;

	/// Enum used by Tree nodes to keep track of the object type
	/// they point to
	enum class ObjectType
	{
		Object,
		Tickable
	};

private:
	/**
	 * Tree data structure which will an outer agent and all its dependent
	 * components
	 */
	struct Tree
	{
		struct Node
		{
			Node() = default;

			/// type of the object this node points to
			ObjectType type{};

			/// Index, pointing to the object in one of the vector buffers
			/// identified by the type of the object in member 'type'.
			size_t element_idx{};

			/// level of this node in the tree
			int level{};

			/// pointer to parent of this node
			Node* parent{};

			/// pointer to the tree that hosts this node
			Tree* host_tree{};

			/// children of this node
			std::vector<Node> children{};
		};

		/**
		 * Construct an empty tree
		 * @param [in] tick_group Assigned tick group of this tree
		 */
		Tree(int tick_group)
				: tick_group(tick_group)
		{};

		/**
		 * Creates the root of the tree and stores the input object in its
		 * respective concept container
		 * @param object Outer pixie object
		 * @return A reference to the root
		 */
		Node& AddRoot(object_type&& object)
		{
			return AddNode(std::move(object), nullptr);
		}

		/**
		 * Creates a node and attaches it to its parent node.
		 * This method also stores the input concept object in its respective
		 * concept container
		 * @param object A pixie object
		 * @param parent Pointer to the parent node of the object
		 * @return A reference to newly created node
		 */
		Node& AddNode(object_type&& object, Node* parent)
		{
			using std::move;

		#ifdef __APPLE_CLANG__
			using mpark::get;
			using mpark::holds_alternative;
		#else
			using std::get;
			using std::holds_alternative;
		#endif

			Node node;
			node.parent = parent;
			node.host_tree = this;

			if (holds_alternative<Object>(object))
			{
				node.type = ObjectType::Object;
				node.element_idx = objects.size();

				auto& vobj = get<Object>(object);
				objects.emplace_back(move(vobj));
			}
			else if (holds_alternative<Tickable>(object))
			{
				node.type = ObjectType::Tickable;
				node.element_idx = tickables.size();

				auto& vobj = get<Tickable>(object);
				tickables.emplace_back(move(vobj));
			}
			else
			{
				// TODO(Ahura): unsupported type
			}

			if (parent)
			{
				node.level = parent->level + 1;
				return parent->children.emplace_back(move(node));
			}
			else
			{
				node.level = 0;
				root = move(node);
				return root;
			}
		}

		void CallBegin()
		{
			// Since pixie allows it and also it doesn't cost much,
			// call Begin for non-tickable objects in hope some of
			// them have implemented it
			for (size_t i = objects.size(); i --> 0;)
				Begin(objects[i]);

			for (size_t i = tickables.size(); i --> 0;)
				Begin(tickables[i]);
		}

		void CallTick()
		{
			for (size_t i = tickables.size(); i --> 0;)
				Tick(tickables[i]);
		}

		void CallEnd()
		{
			// Since pixie allows it and also it doesn't cost much,
			// call End for non-tickable objects in hope some of
			// them have implemented it
			for (size_t i = objects.size(); i --> 0;)
				End(objects[i]);

			for (size_t i = tickables.size(); i --> 0;)
				End(tickables[i]);
		}

		/// Assigned tick group of this tree
		int tick_group  = 0;

		/// Root of the tree
		Node root{};

		/// Registered objects that don't comply with any of the concepts
		std::vector<Object> objects{};

		/// Registered objects that implement Tick concept
		std::vector<Tickable> tickables{};
	};

public:
	/** Default constructor */
	Forest() = default;

	/**
	 * Constructs a new tree with the Outer object T assigned to its root
	 * @tparam T (Required) Type of the object that is being created and
	 * assigned to the root of the new tree
	 * @return A pointer to the newly created object T
	 * @warning Do NOT delete this pointer
	 */
	template<class T>
	T* CreateObject()
	{
		using std::get;
		using std::move;

		// Initialize a new tree for this object and it's components
		auto& tree = trees.emplace_back(Tree(static_cast<int>(trees.size())));

		// Create the object T
		auto [object, object_ptr] = ConstructObject<T>();

		// Move the object to the root of newly initialized tree
		tree.AddRoot(std::move(object));

		// All components of the T should have been initialized and stored
		// by now. Move them from temporary buffers to grow the tree.
		PopulateTree(tree);

		// The dependency tree is formed and all objects are stored in
		// this tree. Clear the temporary buffers and return the pointer
		// to Outer object T
		ClearBuffers();

		return object_ptr;
	}

	/**
	 * Creates a component of type T and stores it in a temporary buffer
	 * @tparam T (Required) Type of the component that is being created
	 * @return A pointer to the created object of type T
	 * @warning Do NOT delete this pointer
	 */
	template<class T>
	T* CreateComponent()
	{
		// TODO(Ahura): We can put a guard (like a counter that has a set
		// limit) to prevent circular object construction. For example if
		// level >= 20, it means we've gone 20 levels deep in dependency
		// construction which is something you don't see everyday... We
		// can then stop the construction here, return a nullptr and warn
		// the user. But first check if compiler itself can identify this
		// circular construction. Also find a reasonable value for the
		// limit.

		// A component is being created. increments the construction level
		// to keep track of the hierarchy of this sub-component
		++component_level;

		// Create the component
		auto [component, component_ptr] = ConstructObject<T>();

		// store it in the temporary buffer
		temp_buffer.emplace_back(std::move(component), component_level);

		// The component is fully constructed. We are now recursing back
		// to the main object, hence we should decrement the level tracker
		--component_level;

		return component_ptr;
	}

	void CallBegin()
	{
		for (auto& tree : trees)
			tree.CallBegin();
	}

	void CallTick()
	{
		for (auto& tree : trees)
			tree.CallTick();
	}

	void CallEnd()
	{
		for (auto& tree : trees)
			tree.CallEnd();
	}

private:
	/**
	 * Constructs an object of T and assigns it to one of the pixie's concepts
	 * based on its interface
	 * @tparam T (Required) Type of the object that is being created
	 * @return A tuple where first element holds the type erased object
	 * represented by object_type variant and second element is a pointer
	 * to the newly created object enclosed within a pixie object.
	 * @warning Do NOT delete the pointer
	 */
	template<class T>
	std::tuple<object_type, T*> ConstructObject()
	{
		using std::move;

		if(HasTick<T>) // implements tick
		{
			Tickable obj = T();
			T* ptr = obj.StaticCast<T>();
			return{move(obj), move(ptr)};
		}
		else
		{
			Object obj = T();
			T* ptr = obj.StaticCast<T>();
			return{move(obj), move(ptr)};
		}
	}

	/**
	 * Populates the tree using the root object components stored in a
	 * temporary buffer
	 * @param [in] tree The tree that will hold all this group of components
	 */
	void PopulateTree(Tree& tree)
	{
		/*
		 * The following algorithm and overall the whole idea of using
		 * this forest to form the construction trees is as follows:
		 * Imagine the following scenario where the outer Agent object
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
		 * main object (which is usually expected to be put directly into
		 * the scene) is queued for construction, the forest first creates
		 * a brand new tree and only then calls the constructor of this
		 * scene object. As the constructed of this object is being
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
		 * the top of the construction hierarchy and to the main object
		 * that started all of this. At this point, the main object's
		 * construction is over and the object is ready. We add this
		 * object to the root of the tree, and attempt to unwrap the
		 * temporary buffer and form the tree using all the dependent
		 * components. This last part is where this function 'PopulateTree' comes into play.
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

		// A stack to keep track of outer object (parent in tree terms)
		std::stack<Tree::Node*> parents;

		// first parent is always the root of the tree
		parents.push(&tree.root);

		while(not temp_buffer.empty())
		{
			auto& object = temp_buffer.back().first;
			auto& level = temp_buffer.back().second;
			auto* parent = parents.top();

			// if the last object in temp_buffer is one level
			// higher than the parent, then this is indeed the
			// correct parent of the object.
			if(not parents.empty() and level > parent->level)
			{
				// Move the object to the tree
				auto& node = tree.AddNode(std::move(object), parent);

				// We're going one level deeper to identify the children
				// of this newly added node
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
	std::vector<Tree> trees;

	/// Tracks the construction level of the object and its components
	int component_level = 0;

	/// Temporary buffer that holds components of the Outer object
	std::vector<std::pair<object_type, int>> temp_buffer;
};

}

#endif //PIXIE_CORE_SCENE_FOREST_H
