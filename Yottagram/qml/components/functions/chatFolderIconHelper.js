function getIcon(iconName) {
    var image = "image://theme/"
    switch (iconName) {
    case "Groups":
        image += "icon-m-users";
        break;
    case "Custom":
        image += "icon-m-file-folder";
        break;
    case "Setup":
        image += "icon-m-events";
        break;
    case "Favorite":
        image += "icon-m-favorite"
        break;
    case "Home":
        image += "icon-m-home";
        break;
    case "Game":
        image += "icon-m-game-controller";
        break;
    case "Mask":
        image += "icon-m-incognito";
        break;
    case "Travel":
        image += "icon-m-airplane-mode";
        break;
    case "Work":
        image += "icon-m-company";
        break;
    case "All":
        image += "icon-m-chat";
        break;
    case "Unread":
        image += "icon-m-message";
        break;
    case "Unmuted":
        image += "icon-m-browser-notifications";
        break;
    case "Bots":
    case "Channels":
    case "Private":
    case "Crown":
    case "Flower":
    case "Love":
    case "Party":
    case "Sport":
    case "Study":
    case "Trade":
    case "Cat":
    default:
        return "image://theme/icon-m-contact"
    }
    return image;
}
