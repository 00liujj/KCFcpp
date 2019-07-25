import cv2
import sys,re

mp4file = sys.argv[1]


txtfile = re.sub(r'\.mp4$', '.txt', mp4file)

txtlst = open(txtfile).readlines()


labeldict = {}

for line in txtlst:
    items = line.split()
    frameid = int(items[0])
    label = items[1]
    box = [float(i) for i in items[2:]]
    labeldict[frameid] = (frameid, label, box)


print(labeldict)


labelmap = {
  '0': 'palm',
  '1': 'fist',
  '2': 'thumb',
  '3': 'vsign'
}


cap = cv2.VideoCapture(mp4file)

count = 0
while True:
    ret, mat = cap.read()
    if not ret:
        break
    count+=1
    imw = mat.shape[1]
    imh = mat.shape[0]
 
    if count in labeldict:
        label = labeldict[count][1]
        box = labeldict[count][2]
        cx = int(box[0] * imw) 
        cy = int(box[1] * imh)
        hw = int(box[2] * imw/2)
        hh = int(box[3] * imh/2)

        cv2.rectangle(mat, (cx-hw, cy-hh), (cx+hw, cy+hh), (255,0,0))
        cv2.putText(mat, labelmap[label], (cx, cy), 0, 1, (255,0,0))
    

    cv2.imshow("hg", mat)
    cv2.waitKey(0)




