# https://github.com/KubaO/stackoverflown/tree/master/questions/py-msg-lark-77687154

ctemplate = """
struct {name} {{
    {name}(const Packet&);
    
    static constexpr const int id={id};
    
    {cmembers}
}};
"""

from lark.visitors import Interpreter

class CGen(Interpreter):
    def message(self, tree):
        self.msg = { "members": {} }
        self.visit_children(tree)
        self.source = CGen._process_message(self.msg)
    
    @staticmethod
    def _process_message(msg):        
        members = ""
        for _name, _type in msg["members"].items():
            if members:
                members += "\n    "
            members += f"{_type} {_name};"
        msg["cmembers"] = members
        
        return ctemplate.format(**msg)
            
    def msgname(self, tree):
        self.msg["name"] = tree.children[0].value
        
    def msgid(self, tree):
        self.msg["id"] = int(tree.children[0].value)
    
    def member(self, tree):
        for child in tree.children:
            if child.type == 'DATATYPE':
                member_type = child.value
            if child.type == 'MEMBER_NAME':
                member_name = child.value
        
        self.msg["members"][member_name] = member_type

from lark import Lark

tree = Lark("""
start: message+
message: msgname msgid member+
msgname: "name" MSG_NAME
msgid: "id" MSG_ID
member: DATATYPE MEMBER_NAME

DATATYPE: "float"|"int"|"bool"
MSG_NAME: WORD
MEMBER_NAME: WORD
MSG_ID: INT

%import common (INT, WORD, WS)
%ignore WS
""").parse("""
name TWIST
id 123
float variableone
float variabletwo
""")

cgen = CGen()
cgen.visit(tree)
print(cgen.source)
