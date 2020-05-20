/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NANVIX_MPUTIL_OBJECT_H_
#define NANVIX_MPUTIL_OBJECT_H_

#include <nanvix/ulib.h>

typedef struct object_t object_t;
typedef struct object_class_t object_class_t;
typedef void (object_construct_t) (object_t *);
typedef void (object_destruct_t) (object_t *);

/**
 * @brief Struct that defines a generic object.
 */
struct object_t
{
    object_class_t *obj_class; /* Object class.              */
    int refcount;              /* Obj reference count.       */
    short is_dynamic;          /* Obj dynamically allocated? */
};

/**
 * @brief Class descriptor.
 *
 * @note There should be a single instance of this descriptor for each class
 * definition.
 */
 struct object_class_t {
    const char *cls_name;              /**< Symbolic name of class. */
    object_construct_t *obj_construct; /**< Object constructor.     */
    object_destruct_t *obj_destruct;   /**< Object destructor.      */
    size_t cls_sizeof;                 /**< Size of an instance.    */
};

/*============================================================================*
 * Class Related Macros.                                                      *
 *============================================================================*/

/**
 * @brief Return a pointer to the class descriptor associated with a class type.
 *
 * @param NAME Name of class.
 *
 * @returns Pointer to class descriptor.
 */
#define OBJ_CLASS(NAME) (&(NAME ## _class))

/**
 * @brief Static initializer for a class descriptor.
 *
 * @param NAME          Name of class
 * @param CONSTRUCTOR   Pointer to constructor
 * @param DESTRUCTOR    Pointer to destructor
 */
#define OBJ_CLASS_INSTANCE(NAME, CONSTRUCTOR, DESTRUCTOR, SIZE)  \
        object_class_t NAME ## _class = {                        \
        # NAME,                                                  \
        (object_construct_t *) CONSTRUCTOR,                      \
        (object_destruct_t *) DESTRUCTOR,                        \
        SIZE                                                     \
    }

/**
 * @brief Declaration for class descriptor.
 *
 * @param NAME Name of class
 */
#define OBJ_CLASS_DECLARATION(NAME)             \
    extern object_class_t NAME ## _class

/**
 * @brief Macro that calls the static object constructor.
 */
#define OBJ_CONSTRUCT(obj, type)    \
    (object_construct((object_t *) obj, OBJ_CLASS(type)))

/**
 * @brief Macro that calls the static object destructor.
 */
#define OBJ_DESTRUCT(obj)    \
    (object_destruct((object_t *) obj))

/**
 * @brief Macro that calls the dynamic object constructor.
 */
#define OBJ_NEW(type)    \
    ((type *)object_new(OBJ_CLASS(type)))

/**
 * @brief Macro that calls the object_retain function.
 */
#define OBJ_RETAIN(obj)    \
    (object_retain((object_t *) obj))

/**
 * @brief Macro that calls the dynamic object desructor.
 */
#define OBJ_RELEASE(obj)    \
    (object_release((object_t **) &obj))

/*============================================================================*
 * Object related functions.                                                  *
 *============================================================================*/

/**
 * @brief Initializes a pre-allocated object.
 *
 * @param object Pre-allocated object reference.
 * @param cls    Object class.
 */
static inline void object_construct(object_t * object, object_class_t * cls)
{
    uassert(object != NULL);
    uassert(cls != NULL);

    object->obj_class  = cls;
    object->refcount   = 1;
    object->is_dynamic = 0;

    cls->obj_construct(object);
}

/**
 * @brief Finishes objects that were not dynamically allocated.
 *
 * @param object Object reference to be finished.
 */
static inline void object_destruct(object_t * object)
{
    uassert(object != NULL);
    uassert(!object->is_dynamic);

    object->obj_class->obj_destruct(object);
}

/**
 * @brief Dynamically allocates storage for an object of @p cls and run
 * its constructor.
 *
 * @param cls Class of the object to be allocated.
 *
 * @returns Upon successful completion, a pointer to the newly allocated
 * object is returned. A NULL pointer is returned instead.
 */
static inline void * object_new(object_class_t * cls)
{
    object_t * new_obj;

    uassert(cls != NULL);

    new_obj = (object_t *) umalloc(cls->cls_sizeof);

    if (new_obj != NULL)
    {
        new_obj->obj_class  = cls;
        new_obj->refcount   = 1;
        new_obj->is_dynamic = 1;
        cls->obj_construct(new_obj);
    }

    return (new_obj);
}

/**
 * @brief Increments the @p object refcount by ONE.
 *
 * @param object Object descriptor to be incremented.
 */
static inline void object_retain(object_t * object)
{
    uassert(object != NULL);
    uassert(object->obj_class != NULL);

    object->refcount++;
}

/**
 * @brief Decrements the @p object refcount by ONE.
 *
 * @param object Object descriptor to be released.
 *
 * @note If the @p object refcount reaches zero, the object is
 * destroyed and its storage space is freed.
 */
static inline void object_release(object_t ** object)
{
    object_t * aux = *object;

    uassert(aux != NULL);
    uassert(aux->obj_class != NULL);

    if (--(aux->refcount) == 0)
    {
        aux->obj_class->obj_destruct(aux);

        if (aux->is_dynamic)
        {
            ufree((void *) aux);
            *object = NULL;
        }
    }
}

#endif /* NANVIX_MPUTIL_OBJECT_H_ */
