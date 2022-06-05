@0x87d8513e0d4d896f;

annotation structAnnotation(struct): Text;
annotation fieldAnnotation(field): Text;

struct Struct $structAnnotation("struct") {
	field @0 :Struct2 $fieldAnnotation("field");
}

struct Struct2 $structAnnotation("def") {
	field @0 :UInt32;
}
