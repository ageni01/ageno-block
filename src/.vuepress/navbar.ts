import { navbar } from "vuepress-theme-hope";

export default navbar([
  "/",
  { text: "文章导航", icon: "book", link: "/posts/" },
  { text: "关于", icon: "circle-info", link: "/intro.html" },
]);
