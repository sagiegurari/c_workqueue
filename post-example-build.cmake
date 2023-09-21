
include("./cmake-modules/src/utils.cmake")

utils_embed_example_source_in_readme(
  EXAMPLE_FILE "../examples/example.c"
  DOCUMENT_FILE "../README.md"
  SOURCE_TYPE "c"
)

