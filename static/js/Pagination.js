/*2019年5月9日，作者:Asciphx*/
function Pagination(paramsObj){
    this.init(paramsObj);
}
Pagination.prototype={
    state:{current: 1,pageCount:1,maxShow:1},
    init:function(paramsObj){
        var state = this.state
        // 页面元素的外部容器
        state.container = paramsObj.container || '.page'
        // 总页数量参数
        state.pageCount = paramsObj.pageCount>1?paramsObj.pageCount:1
        // 初始页参数
        state.current = paramsObj.current>1?paramsObj.current:1
        // 中间向左右两边延申多展示出的按钮数
        state.maxShow = paramsObj.maxShow>1?paramsObj.maxShow:1
        // 回调函数
        state.onPageChange = paramsObj.onPageChange
        // 当选中页码时添加的类名class
        state.activeCName = paramsObj.activeCName || 'current'
        // 代表页码数字的属性
        state.dataNumberAttr = paramsObj.dataNumberAttr || 'tcdNumber'
        // 上一页 按钮的类名class
        state.prevCName = paramsObj.prevCName || 'prevPage'
        // 下一页 按钮的类名class
        state.nextCName = paramsObj.nextCName || 'nextPage'
        this.renderPageDOM(state)
    },
    switchPage:function(){
        var state = this.state,g=this
        // 所有的页码元素a，包括上一页、下一页
        var pCNameList = this.selectorEle(state.container+" a", true)
        var current
        for(i in pCNameList){
            if(i<pCNameList.length)
                pCNameList[i].addEventListener("click",function() {
                    if (this.className==state.activeCName) return
                    var dataNumberAttr = this.innerHTML
                    if (dataNumberAttr=="⇇"||dataNumberAttr=="⇉") {
                        current = parseInt(this.className.split(" ")[1])
                    } else if (this.className==state.prevCName) {
                        state.current > 1 && (current = state.current - 1)
                    } else if (this.className==state.nextCName) {
                        state.current < state.pageCount && (current = state.current + 1)
                    } else {
                        current = parseInt(dataNumberAttr)
                    }
                    current && g.gotoPage(current)
                })
        }
    },
    renderPageDOM:function(args) {
        var pageContainer = this.selectorEle(args.container)
        var paginationStr = '',start,end
        if (args.current > 1) {
            paginationStr='<a href = "javascript:;" class="prevPage">←</a>';
        } else {
            paginationStr='<a class="disabled">←</a>';
        }
        if(args.pageCount<(4+2*args.maxShow)){
            // 总页数小于6的时候直接遍历。
            for(start=0;start<args.pageCount;start++){
                end=start+1;
                if (end == args.current) {
                    paginationStr += '<a class="current">' + end + '</a>';
                } else {
                    paginationStr += '<a href = "javascript:;" class="tcdNumber">' + end + '</a>';
                }
            }
        }else{
            // 下面代码有点绕，请勿尝试阅读
            start = args.current - args.maxShow;end = args.current + args.maxShow;
            if (args.current >1+args.maxShow) {paginationStr += '<a href = "javascript:;" class="tcdNumber">' + 1 +
            '</a>';}else if(args.pageCount>2+2*args.maxShow){end=2+2*args.maxShow};
            if(args.current >args.pageCount-2-args.maxShow && args.pageCount-2-args.maxShow>0){start=args.pageCount-1-2*args.maxShow;}
            if (args.current >2+args.maxShow&&start-1!=1) {paginationStr += '<a href = "javascript:;" class="tcdNumber '+(start-1)+
            '" style="width:9px" onmouseover="this.innerHTML=\'⇇\'" onmouseout="this.innerHTML=\'...\'">...</a>';}
            // 中间页数利用循环生成
            for (; start <= end; start++) {
                if (start <= args.pageCount && start >0) {
                    if (start == args.current) {
                        paginationStr += '<a class="current">' + start + '</a>';
                    } else {
                        paginationStr += '<a href = "javascript:;" class="tcdNumber">' + start + '</a>';
                    }
                }
            }
            // 判断临界值插入省略号
            if (args.current < args.pageCount - 1 - args.maxShow&&end+1!=args.pageCount) {
                paginationStr += '<a href = "javascript:;" class="tcdNumber '+(end+1)+
                '" style="width:9px" onmouseover="this.innerHTML=\'⇉\'" onmouseout="this.innerHTML=\'...\'">...</a>';
            }
            // 将中间数值插入html内容中
            if (args.current < args.pageCount - args.maxShow) {
                paginationStr += '<a href="javascript:;" class="tcdNumber">' + args.pageCount + '</a>';
            }
        }
        if (args.current < args.pageCount) {
            paginationStr += '<a href = "javascript:;" class="nextPage">→</a>';
        } else {
            paginationStr += '<a class="disabled">→</a>';
        }
        pageContainer.innerHTML = paginationStr
        this.switchPage()
    },
    gotoPage:function(current) {
        this.state.current = current
        this.renderPageDOM(this.state)
        this.state.onPageChange(current)
    },
    hasClass:function(eleObj, className) {
        return eleObj.classList.contains(className);
    },
    selectorEle:function(selector, all) {
        return all ? document.querySelectorAll(selector) : document.querySelector(selector)
    }
}