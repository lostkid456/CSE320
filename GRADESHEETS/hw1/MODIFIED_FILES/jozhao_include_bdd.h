--- hw1/dev_repo/basecode/hw1/include/bdd.h

+++ hw1/repos/jozhao/hw1/include/bdd.h

@@ -47,12 +47,12 @@

 BDD_NODE bdd_nodes[BDD_NODES_MAX];

 

 /*

- * Open-addressed hash map mapping triples (v, l, r), where l and r are

- * BDD node indices and v is a level number, to BDD node indices.

- * Each node can be present at most once, so there will be at most

- * BDD_NODES_MAX entries in the table.  We define the table size to yield

- * a load factor of no more than 0.5; in particular the table will never

- * be full.  So the table size must be a prime number >= 2 * BDD_NODES_MAX.

+ * Open-addressed hash map mapping pairs (l, r) of BDD node indices

+ * to BDD node indices.  Each node can be present at most once, so

+ * there will be at most BDD_NODES_MAX entries in the table.

+ * We define the table size to yield a load factor of no more than 0.5;

+ * in particular the table will never be full.  So the table size must

+ * be a prime number >= 2 * BDD_NODES_MAX.

  */

 #define BDD_HASH_SIZE 2097169

 BDD_NODE *bdd_hash_map[BDD_HASH_SIZE];

@@ -198,23 +198,18 @@

 

 /**

  * Given a BDD node that represents a 2^d x 2^d image, construct a new

- * BDD node that represents the result of "zooming" by a specified

- * nonnegative "factor" k.  "Zooming in" is an operation that increases

- * the number of pixels by replacing each pixel in the original image

+ * BDD node that represents the result of "zooming in" by a specified

+ * nonnegative "factor" k.  Zooming in is an operation that increases

+ * the number of pixels by replacing each pixel in the individual image

  * by a 2^k x 2^k array of identical pixels in the transformed image.

- * "Zooming out" is an operation that decreases the number of pixels

- * by replacing each 4x4 square of pixels in the original image by a

- * single pixel.  As discussed in the assignment handout, this single

- * pixel should be a black pixel if the 4x4 square is entirely black,

- * otherwise it is a white pixel.  Zooming by a factor of 0 is an

- * identity transformation.

+ * Zooming by a factor of 0 is an identity transformation.

  *

  * @param node  The BDD node to transform.

  * @param level  The level at which to interpret the node,

  * which (due to "skipped levels") might be larger than the level

  * recorded in the node itself.

- * @param factor   The "zoom factor", which must be a value

- * satisfying -BDD_LEVELS_MAX <= d+k <= BDD_LEVELS_MAX.

+ * @param factor   The "zoom factor", which must be a nonnegative value

+ * satisfying 0 <= d+k <= BDD_LEVELS_MAX.

  * @return  the BDD node that represents the result of the "zoom"

  * operation.

  */
