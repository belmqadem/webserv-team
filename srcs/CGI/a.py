
class A:
    def __init__(self, a,s,d):
        self.a = a
        self.s =s
        self.d = d
    def __str__(self):
       return print("ssssssss  " + self.a  +" nn ")




s = A("habb", 12, 34.4)
s.__str__()