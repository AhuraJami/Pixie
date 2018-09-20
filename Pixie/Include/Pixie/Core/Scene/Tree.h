#ifndef PIXIE_TREE_H
#define PIXIE_TREE_H

#ifdef __APPLE_CLANG__
#include <mpark/variant.hpp>
#else
#include <variant>
#endif

#include <deque>
#include <vector>
#include <algorithm>

#include <Pixie/Concepts/PObject.h>
#include "Pixie/Misc/PixieExports.h"

namespace pixie
{

/**
 * Tree data structure which holds an outer agent and all its dependent
 * components
 */
struct Tree
{
	struct Node
	{
		Node() = default;

		/// pointer to the tree that hosts this node
		Tree* host_tree{};

		PObject data{};

		/// Index, pointing to the object in one of the containers
		size_t element_idx{};

		/// Index of the container the object of this node is held in
		int list_idx{};

		/// level of this node in the tree
		int level{};

		/// pointer to parent of this node
		Node* parent{};

		/// children of this node
		std::vector<Node> children{};
	};

public:
	/** Default constructor */
	Tree() = default;

	/**
	 * Construct an empty tree
	 * @param [in] tick_group Assigned tick group of this tree
	 */
	explicit Tree(int tick_group)
			: tick_group(tick_group)
	{}

	/**
	 * Creates the root of the tree and stores the input object in its
	 * respective concept container
	 * @param [in] object Entity object contained in the pixie object
	 */
	void AddRoot(PObject entity)
	{
		AddNode(std::move(entity), nullptr);
	}

	/**
	 * Creates a node and attaches it to its parent node.
	 * This method also stores the input concept object in its respective
	 * concept container
	 * @param [in] object A pixie object
	 * @param [in] parent Pointer to the parent node of the object
	 * @param [in] Pointer
	 * @return A reference to newly created node
	 */
#ifdef __APPLE_CLANG__
	Node& AddNode(mpark::variant<PObject, PObject*> obj, Node* parent)
	{
		using mpark::get_if;
#else
	Node& AddNode(std::variant<PObject, PObject*> obj, Node* parent)
	{
		using std::get_if;
#endif
		using std::move;

		Node node;

		if (PObject* pobject = get_if<PObject>(&obj)) // Constructing a type T component
		{
			// Move and store its contained data into one of the available lists
			// and let the pobject itself go out of scope
			MovePObjectDataToList(move(*pobject), &node);
		}
		else if (PObject** ptr = get_if<PObject*>(&obj)) // Constructing a pure PObject
		{
			node.list_idx= 3;
			node.element_idx = pobjects.size();
			pobjects.emplace_back(*ptr);
		}

		// Fill out the remaining fields of the node
		node.parent = parent;
		node.host_tree = this;

		if (parent != nullptr)
		{
			node.level = parent->level + 1;
			parent->children.emplace_back(node);
			return parent->children.back();
		}
		else
		{
			node.level = 0;
			root = move(node);
			return root;
		}
	}

	/**
	 * Moves the data hold by the PObject to its appropriate container
	 * @param pobject The PObject holding the data
	 * @param node The tree node that the PObject's data will link to
	 */
	void MovePObjectDataToList(PObject pobject, Node* node)
	{
#ifdef __APPLE_CLANG__
		using mpark::get;
		using mpark::holds_alternative;
#else
		using std::get;
		using std::holds_alternative;
#endif
		using std::move;

		if (holds_alternative<Object>(pobject.data))
		{
			node->list_idx = 0;
			node->element_idx = objects.size();
			objects.emplace_back(move(get<Object>(pobject.data)));
		}
		else if (holds_alternative<Tickable>(pobject.data))
		{
			node->list_idx = 1;
			node->element_idx = tickables.size();
			tickables.emplace_back(move(get<Tickable>(pobject.data)));
		}
		else
		{
			// TODO(Ahura): unsupported type
			node->list_idx = -1;
			node->element_idx = 0;
		}
	}

	// TODO(Ahura): Need to reverse all of the nodes' element_index
	/**
	 * Reverses the container at the end of the tree construction
	 * So that we don't have to reverse iterate over them
	 */
	void ReverseContainers()
	{
		std::reverse(objects.begin(), objects.end());
		std::reverse(tickables.begin(), tickables.end());
		std::reverse(pobjects.begin(), pobjects.end());
	}

	/**
	 * Calls the Begin method of the registered Objects
	 */
	void CallBegin()
	{
		// Since pixie allows it and also it doesn't cost much,
		// call Begin for non-tickable objects in hope some of
		// them have implemented it
		for (auto& obj : pobjects)
			Begin(*obj);

		for (auto& obj : objects)
			Begin(obj);

		for (auto& tick_obj : tickables)
			Begin(tick_obj);
	}

	/**
	 * Calls the Tick method of the registered Objects
	 */
	inline void CallTick()
	{
		for (auto& obj : pobjects)
			Tick(*obj);

		for (auto& tick_obj : tickables)
			Tick(tick_obj);
	}

	/**
	 * Calls the End method of the registered Objects
	 */
	void CallEnd()
	{
		// Since pixie allows it and also it doesn't cost much,
		// call End for non-tickable objects in hope some of
		// them have implemented it
		for (auto& obj : pobjects)
			End(*obj);

		for (auto& obj : objects)
			End(obj);

		for (auto& tick_obj : tickables)
			End(tick_obj);
	}

	/// Assigned tick group of this tree
	int tick_group = 0;

	/// Root of the tree
	Node root{};

	/// Registered objects that don't comply with any of the concepts
	std::vector<Object> objects{};

	/// Registered objects that implement Tick concept
	std::vector<Tickable> tickables{};

	/// Pointer to registered PObjects owned by and held in its outer class
	std::vector<PObject*> pobjects{};
};

} // namespace pixie

#endif //PIXIE_TREE_H
