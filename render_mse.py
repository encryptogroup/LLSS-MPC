import sys
d = 768
l = 64

with open("Circuits/mse.txt", "w") as file:
    # mean ~y
    file.write("INPUT:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < Blinded:Arithmetic > { Blinded:Blinded } | 0\n")
    # samples y_i
    a = 1
    for i in range(d):
        file.write("INPUT:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Blinded } | "+str(a+i)+"\n")
    x = d+1
    # invert samples y_i
    for i in range(d):
        file.write("*:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Arithmetic } c-1 "+str(a+i)+" | "+str(x+i)+"\n")
    y = 2*d+1
    # ~y - y_i
    for i in range(d):
        file.write("+:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Arithmetic } 0 "+str(x+i)+" | "+str(y+i)+"\n")
    # square
    z= 3*d + 1
    for i in range(d):
        file.write("*:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Arithmetic } "+str(y+i)+" "+str(y+i)+" | "+str(z+i)+"\n")

    h = 4*d + 1
    # sum
    for i in range(d-1):
        w = h+i-1
        if i == 0:
            w = z
        file.write("+:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Arithmetic } "+str(w)+" "+str(z+i+1)+" | "+str(h+i)+"\n")

    o = 5*d - 1
    # mult with const
    file.write("*:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Blinded:Arithmetic } "+str(o)+" c42 | "+str(o+1)+"\n")
    file.write("OUTPUT:"+str(l)+",Scalar,Integer>"+str(l)+",Scalar,Integer < > { Arithmetic:Arithmetic } "+str(o+1)+" | "+str(o+2)+"\n")
    

print("Created mse.txt GTN.")
