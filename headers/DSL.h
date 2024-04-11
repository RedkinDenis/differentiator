#ifndef DSL_H
#define DSL_H

#define GET_VARIABLE_NAME(variable) #variable

#define SYNTAX_ERROR printf("SYNTAX ERROR!!!\n");

#define REQUIRE(r)                                      \
    do                                                  \
    {                                                   \
        if (**s == r)                                   \
            *s += 1;                                    \
        else                                            \
        {                                               \
            SYNTAX_ERROR                                \
            printf("CALLED FROM LINE: %d\n", __LINE__); \
        }                                               \
    } while (0)             

#define CHANGE_NODE(from, to)        \
    do                               \
    {                                \
    Node* tree_temp_ = 0;            \
    CALLOC(to, Node, 1);             \
    tree_temp_ = from;               \
    from = to;                       \
    from->parent = tree_temp_;       \
    level++;                         \
    } while(0)


#define FREE_SUBTREE(subtree)   \
    do                          \
    {                           \
    free(subtree);              \
    subtree = NULL;             \
    } while (0)


#define DEFUALT_NODE create_node(DEFUALT, NULL, NULL, NULL)
#define DIFF_LEFT  diff(node->left, part)
#define DIFF_RIGHT diff(node->right, part)
#define COPY_LEFT  copy_subtree(node->left)
#define COPY_RIGHT copy_subtree(node->right)

#endif // DSL_L