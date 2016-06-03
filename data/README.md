## Data Dir

### img
- save the test remote image, input and output

### log
- main to record the system run info, about the input parameters and output parameters

### serialize
- main to save the finished task info, about the input paramters and output parameters
- can save two copy, one is string text file(about json), the others about binary file(about json)
- the file format maybe

```bash
{
	"task_id1" : [{}, {}],
	"task_id2" : [{}, {}],
	......
}
```
