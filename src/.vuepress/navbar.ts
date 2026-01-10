import { navbar } from "vuepress-theme-hope";

export default navbar([
  "/",
  "/demo/",
  {
    text: "博文",
    icon: "pen-to-square",
    prefix: "/posts/",
    children: [
      {
        text: "香蕉",
        icon: "pen-to-square",
        prefix: "banana/",
        children: [
          {
            text: "香蕉 1",
            icon: "pen-to-square",
            link: "1",
          },
          {
            text: "香蕉 2",
            icon: "pen-to-square",
            link: "2",
          },
          "3",
          "4",
        ],
      },
      "tomato",
      {
        text: "虚幻",
        icon: "pen-to-square",
        prefix: "unreal/",// 子菜单
        children: [
          {
            text: "基础知识1",
            icon: "pen-to-square",
            link: "BasicUC++", // 链接,或者md的名称
          },
        ],
      },
      {
        text: "java",
        icon: "pen-to-square",
        prefix: "java/",// 子菜单
        children: [
          {
            text: "多线程-暂无",
            icon: "pen-to-square",
            link: "Lern", // 链接,或者md的名称
          },
        ],
      }
    ],
  }
]);
