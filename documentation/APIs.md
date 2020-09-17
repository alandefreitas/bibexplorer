# APIs for bibliometric information

This is a list of APIs that provide bibliometric information. 

| API       | Microsoft    |
|-----------|--------------|
| Website   | [URL](https://docs.microsoft.com/en-us/academic-services/project-academic-knowledge/introduction)  |
| Register  | [URL](https://www.microsoft.com/en-us/research/project/academic-knowledge/) |
| Documentation | [URL](https://dev.elsevier.com/api_docs.html) |
|  Free | ✅ |
|  DOI   | ❌ |
|  #Citations   | ✅ |
|  Citation IDs   | ❌ |
|  #References   | ❌ |
|  Reference IDs   | ❌ |
|  Journal Indicators | ❓ |
|  Data Format   | JSON |
|  Limitations   | Not many papers |

# Examples 

## Microsoft academic

Get paper information: 

```html
https://api.labs.cognitive.microsoft.com/academic/v1.0/evaluate?expr=And(Composite(AA.AuN=='{}'),Y=[{},{}],Ti='{}',Ty='{}')&model=latest&count=10&offset=0&attributes=Id,Ty,Ti,Y,CC,CitCon,ECC,AA.AfId,AA.AfN,AA.AuId,AA.AuN,AA.DAuN,AA.DAfN,AA.S,AW,BT,C.CId,C.CN,D,DN,DOI,F.DFN,F.FId,F.FN,FamId,FP,I,J.JId,J.JN,LP,PB,Pt,RId,S,Ti,V,VFN,VSN,W,Y
```
