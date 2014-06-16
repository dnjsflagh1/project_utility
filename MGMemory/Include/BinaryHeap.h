/******************************************************************************/
#ifndef __BINARYHEAP_H__
#define __BINARYHEAP_H__
/******************************************************************************/


/*****************************************************************************
*                                    binaryheap.h
*
* Minimum binary heap implemented by C++ template class.
*
* This class provides the following operatins of a minimum binary heap:
*      build a heap
*      insert an element into the heap
*      find the minimum element in the heap
*      delete the minimum element in the heap
*
* The defualt initial size of the heap is set to 20. If the elements number
* exceed initial size, then it will be extended by a factor of 2.
*
* Zhang Ming, 2009-10
*****************************************************************************/

#include <iostream>
#include <cstdlib>

using namespace std;

namespace MG
{
    #define BINARYHEAP_INITSIZE 1000
    #define BINARYHEAP_EXTFACTOR 2
    
    template <typename Type>
    class BinaryHeap
    {
    public:
        explicit BinaryHeap( int maxSize = BINARYHEAP_INITSIZE );
        BinaryHeap( Type *array, int length );
        ~BinaryHeap();

        inline	bool isEmpty() const;
        inline	void makeEmpty();

        inline	void insert( Type &x );
        inline	void findMin( Type &x );
        inline	void deleteMin();
        inline	void deleteMin( Type &minItem );

		inline	void filterDown( int hole );
		inline	void filterUp( int hole );

    private:

        Type    *elements;

		int	addUpSize;
        int currentSize;
        int capacity;

        inline	void handleOverflow();
        inline	void handleUnderflow();

    };


    /**
    * constructors and destructor
    */
    template <typename Type>
    BinaryHeap<Type>::BinaryHeap( int maxSize )
    {
        capacity	= maxSize;
		addUpSize	= capacity;

        elements = new Type[capacity+1];
        if( elements == NULL )
            cerr << "Out of memory!" << endl;

        currentSize = 0;
    }

    template <typename Type>
    BinaryHeap<Type>::BinaryHeap( Type *array, int length )
    {
        capacity = ( BINARYHEAP_INITSIZE > length ) ? BINARYHEAP_INITSIZE : length;
		addUpSize = length;

        elements = new Type[capacity+1];
        if( elements == NULL )
            cerr << "Out of memory!" << endl;

        for( int i=0; i<length; ++i )
            elements[i+1] = array[i];

        currentSize = length;
        for( int i=currentSize/2; i>0; --i )
            filterDown( i );
    }

    template <typename Type>
    BinaryHeap<Type>::~BinaryHeap()
    {
        currentSize = 0;
        capacity = BINARYHEAP_INITSIZE;
		addUpSize = BINARYHEAP_INITSIZE;
        delete []elements;
    }


    /**
    * If the heap is empty, return true.
    */
    template <typename Type>
    inline bool BinaryHeap<Type>::isEmpty() const
    {
        return currentSize == 0;
    }


    /**
    * Make the heap empty.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::makeEmpty()
    {
        currentSize = 0;
    }


    /**
    * Insert item x, allowing duplicates.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::insert( Type &x )
    {
        if( currentSize == capacity )
            handleOverflow();

		x->binaryHeapIndex = currentSize + 1;
        elements[++currentSize] = x;
        filterUp( currentSize );
    }


    /**
    * Find the smallest item in the heap.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::findMin( Type &x )
    {
        if( !isEmpty() )
            x = elements[1];
        else
            handleUnderflow();
    }


    /**
    * Remove the minimum item.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::deleteMin()
    {
        if( !isEmpty() )
        {
            elements[1] = elements[currentSize--];
			elements[1]->binaryHeapIndex = 1;
            filterDown( 1 );
        }
        else
            handleUnderflow();
    }


    /**
    * Remove the minimum item and place it in minItem.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::deleteMin( Type &minItem )
    {
        if( !isEmpty() )
        {
            minItem = elements[1];
            elements[1] = elements[currentSize--];
            filterDown( 1 );
        }
        else
            handleUnderflow();
    }


    /**
    * Percolate down the heap, begin at "hole".
    */
    template <typename Type>
    inline	void BinaryHeap<Type>::filterDown( int hole )
    {
        int child;
        Type tmp = elements[hole];

        for( ; 2*hole<=currentSize; hole=child )
        {
            child = 2*hole;

            if( child != currentSize && elements[child+1]->binaryHeapCost < elements[child]->binaryHeapCost )
                child++;

            if( elements[child]->binaryHeapCost < tmp->binaryHeapCost )
			{
                elements[hole] = elements[child];
				elements[hole]->binaryHeapIndex = hole;
			}
            else
                break;
        }

        elements[hole] = tmp;
		tmp->binaryHeapIndex = hole;
    }


    /**
    * Percolate up the heap, begin at "hole".
    */
    template <typename Type>
    inline	void BinaryHeap<Type>::filterUp( int hole )
    {
        Type tmp = elements[hole];

        for( ; hole>1 && tmp->binaryHeapCost < elements[hole/2]->binaryHeapCost; hole/=2 )
		{
            elements[hole] = elements[hole/2];
			elements[hole]->binaryHeapIndex = hole;
		}

        elements[hole] = tmp;
		tmp->binaryHeapIndex = hole;
    }


    /**
    * If the capability of the heap exceeds the initial size, make it double.
    */
    template <typename Type>
    inline	void BinaryHeap<Type>::handleOverflow()
    {
        capacity += BINARYHEAP_EXTFACTOR * addUpSize;

        Type *newArray = new Type[capacity+1];
        if( newArray == NULL )
        {
            cerr << "Out of memory!" << endl;
            exit(1);
        }

        for( int i=1; i<=currentSize; ++i )
            newArray[i] = elements[i];

        delete []elements;
        elements = newArray;
    };


    /**
    * Handle the error of get element from an empty heap.
    */
    template <typename Type>
    inline void BinaryHeap<Type>::handleUnderflow()
    {
        cerr << "The heap is empty!" << endl << endl;
        exit( 1 );
    };
}


#endif
// BINARYHEAP_H

