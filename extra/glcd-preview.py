def as_bin(b, l=8):
    byts = bin(b)[2:]
    return "0" * max(l-len(byts), 0) + byts

part = input("Char 5 bytes > ")
strings = list( # back to a list
    filter(lambda l: len(l) > 0, map( # non-empty strings
        lambda l: l.strip(), # stripping string
            part.split(",") # splitting by comma
    ))
)
numbers = list(map(lambda l: int(l, 16), strings)) # parsing as hex, 0xAB
v_bytes = list(map(as_bin, numbers))
h_bytes = list(zip(*v_bytes))
replaced = list(map(lambda l: "".join(l).replace("0", "  ").replace("1", "██"), h_bytes))[::-1]
print("\n".join(replaced))