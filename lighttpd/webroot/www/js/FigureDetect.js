function LongPressDetect(element, longPresscallBack, duration) 
{
  let touchStart, timeout,downtime;
  element.on('touchstart', function(event) {
    console.log("touchstart: " + event.touches.length);
    if(event.touches.length > 1)
    {
      clearTimeout(timeout);
      return;
    }
    downtime = Date.now();
    touchStart = event.originalEvent.changedTouches[0].clientX;
    timeout = setTimeout(function() {
        longPresscallBack();
    }, duration);
  });
  element.on('touchmove', function(event) {
    //console.log("touchmove");
    // 如果移动超过一定距离，则不触发长按
    var curX = event.originalEvent.changedTouches[0].clientX;
    //console.log(Math.abs(curX - touchStart));
    if (Math.abs(curX - touchStart) > 10) 
    {
      clearTimeout(timeout);
    }
  });
 
  element.on('touchend', function(event) {
    //console.log("touchend");
    clearTimeout(timeout);
  });
  element.on('mousedown', function(e) {
    // console.log(e);
    // console.log("mousedown:" + e.pageX);
    downtime = Date.now();
    touchStart = e.pageX;
    timeout = setTimeout(function() 
    {
        longPresscallBack();
    }, duration);
  });
  
  element.on('mousemove', function(e) {
    // console.log(e);
    // console.log("mousemove:" + e.pageX);
    // 如果移动超过一定距离，则不触发长按
    if (Math.abs(e.pageX - touchStart) > 10) 
    {
      clearTimeout(timeout);
    }
  });
 
  element.on('mouseup', function(e) {
    // console.log(e);
    // console.log("mouseup:" + e.pageX);
    clearTimeout(timeout);
  });
}

function FigureDetect(element, longPresscallBack, duration, swipecallback, swipewidth) 
{
  let touchStart, timeout,downtime;
  element.on('touchstart', function(event) {
    console.log("touchstart");
    downtime = Date.now();
    touchStart = event.originalEvent.changedTouches[0].clientX;
    timeout = setTimeout(function() {
        longPresscallBack();
    }, duration);
  });
 
  element.on('touchmove', function(event) {
    console.log("touchmove");
    // 如果移动超过一定距离，则不触发长按
    var curX = event.originalEvent.changedTouches[0].clientX;
    if (Math.abs(curX - touchStart) > 10) 
    {
      clearTimeout(timeout);
    }
  });
 
  element.on('touchend', function(event) {
    console.log("touchend");
    clearTimeout(timeout);
    console.log(event);
    var curX = event.originalEvent.changedTouches[0].clientX;
    if (curX - touchStart > swipewidth) 
    {
        if((Date.now() - downtime) > 200)
        {
            return;
        }
        swipecallback("prev");
    }
    if (touchStart - curX > swipewidth) 
    {
        if((Date.now() - downtime) > 200)
        {
            return;
        }
        swipecallback("next");
    }
  });

  element.on('mousedown', function(e) {
    downtime = Date.now();
    console.log("mousedown");
    touchStart = e.pageX;
    timeout = setTimeout(function() 
    {
        longPresscallBack();
    }, duration);
  });
 
  element.on('mousemove', function(e) {
    console.log("mousemove");
    // 如果移动超过一定距离，则不触发长按
    if (Math.abs(e.pageX - touchStart) > 10) 
    {
      clearTimeout(timeout);
    }
  });
 
  element.on('mouseup', function(e) {
    console.log("mouseup");
    clearTimeout(timeout);
    if (e.pageX - touchStart > swipewidth) 
    {
        if((Date.now() - downtime) > 200)
        {
            return;
        }
        swipecallback("prev");
    }
    if (touchStart - e.pageX > swipewidth) 
    {
        if((Date.now() - downtime) > 200)
        {
            return;
        }
        swipecallback("next");
    }
  });
}